/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "osystem.h"
#include "backends/plugins/wii/wii-provider.h"

#include <ogc/machine/processor.h>
#include <fat.h>
#ifndef GAMECUBE
#include <wiiuse/wpad.h>
#endif
#ifdef USE_WII_DI
#include <di/di.h>
#endif
#ifdef DEBUG_WII_GDB
#include <debug.h>
#endif
#include <gxflux/gfx_con.h>

#ifdef __cplusplus
extern "C" {
#endif

bool reset_btn_pressed = false;
bool power_btn_pressed = false;

void reset_cb(void) {
#ifdef DEBUG_WII_GDB
	printf("attach gdb now\n");
	_break();
#else
	reset_btn_pressed = true;
#endif
}

void power_cb(void) {
	power_btn_pressed = true;
}

static void show_console(int code) {
	u32 i, b;

	printf("ScummVM exited abnormally (%d).\n", code);

	gfx_frame_abort();
	gfx_init();

	if (!gfx_frame_start())
		return;

	gfx_con_draw();
	gfx_frame_end();

	for (i = 0; i < 60 * 3; ++i)
		VIDEO_WaitVSync();

#ifdef DEBUG_WII_GDB
	printf("attach gdb now\n");
	_break();
#endif

	printf("Press any key to continue.\n");

	if (!gfx_frame_start())
		return;

	gfx_con_draw();
	gfx_frame_end();
	VIDEO_WaitVSync();

	while (true) {
		b = 0;

		if (PAD_ScanPads() & 1)
			b = PAD_ButtonsDown(0);

#ifndef GAMECUBE
		WPAD_ScanPads();
		if (WPAD_Probe(0, NULL) == WPAD_ERR_NONE)
			b |= WPAD_ButtonsDown(0);
#endif

		if (b)
			break;

		VIDEO_WaitVSync();
	}

	printf("\n\nExiting...\n");

	if (!gfx_frame_start())
		return;

	gfx_con_draw();
	gfx_frame_end();
	VIDEO_WaitVSync();
}

s32 reset_func(s32 final) {
	static bool done = false;

	if (!done) {
		show_console(-127);
		done = true;
	}

	return 1;
}

static sys_resetinfo resetinfo = {
	{ NULL, NULL },
	reset_func,
	1
};

#ifdef DEBUG_WII_MEMSTATS
void wii_memstats(void) {
	static u32 min_free = UINT_MAX;
	static u32 temp_free;
	static u32 level;

	_CPU_ISR_Disable(level);
#ifdef GAMECUBE
	temp_free = (u32) SYS_GetArenaHi() - (u32) SYS_GetArenaLo();
#else
	temp_free = (u32) SYS_GetArena1Hi() - (u32) SYS_GetArena1Lo() +
				(u32) SYS_GetArena2Hi() - (u32) SYS_GetArena2Lo();
#endif
	_CPU_ISR_Restore(level);

	if (temp_free < min_free) {
		min_free = temp_free;
		fprintf(stderr, "free: %8u\n", min_free);
	}
}
#endif

int main(int argc, char *argv[]) {
	s32 res;

#if defined(USE_WII_DI) && !defined(GAMECUBE)
	DI_Init();
#endif

	VIDEO_Init();
	PAD_Init();
	DSP_Init();
	AUDIO_Init(NULL);

	gfx_video_init(NULL);
	gfx_init();
	gfx_con_init(NULL);

#ifdef DEBUG_WII_GDB
	DEBUG_Init(GDBSTUB_DEVICE_USB, 1);
#endif

	printf("startup as ");
	if (argc > 0)
		printf("'%s'\n", argv[0]);
	else
		printf("<unknown>\n");

	SYS_RegisterResetFunc(&resetinfo);

	SYS_SetResetCallback(reset_cb);
#ifndef GAMECUBE
	SYS_SetPowerCallback(power_cb);
#endif

	if (!fatInitDefault()) {
		printf("fatInitDefault failed\n");
	} else {
		// set the default path if libfat couldnt set it
		// this allows loading over tcp/usbgecko
		char cwd[MAXPATHLEN];

		if (getcwd(cwd, MAXPATHLEN)) {
			size_t len = strlen(cwd);

			if (len > 2 && (cwd[len - 1] == ':' || cwd[len - 2] == ':')) {
				printf("chdir to default\n");
				chdir("/apps/scummvm");
			}
		}
	}

	g_system = new OSystem_Wii();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new WiiPluginProvider());
#endif

	res = scummvm_main(argc, argv);
	g_system->quit();

	printf("shutdown\n");

	SYS_UnregisterResetFunc(&resetinfo);
	fatUnmountDefault();

	if (res)
		show_console(res);

	if (power_btn_pressed) {
		printf("shutting down\n");
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	}

	printf("reloading\n");

	gfx_con_deinit();
	gfx_deinit();
	gfx_video_deinit();

	return res;
}

#ifdef __cplusplus
}
#endif
