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
 *
 */

// Disable printf override in common/forbidden.h to avoid
// clashes with pspdebug.h from the PSP SDK.
// That header file uses
//   __attribute__((format(printf,1,2)));
// which gets messed up by our override mechanism; this could
// be avoided by either changing the PSP SDK to use the equally
// legal and valid
//   __attribute__((format(__printf__,1,2)));
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the PSP port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#define	USERSPACE_ONLY	//don't use kernel mode features

#ifndef USERSPACE_ONLY
#include <pspkernel.h>
#include <pspdebug.h>
#else
#include <pspuser.h>
#endif

#include <psppower.h>

#include <common/system.h>
#include <engines/engine.h>
#include <base/main.h>
#include <base/plugins.h>
#include "backends/platform/psp/powerman.h"
#include "backends/platform/psp/thread.h"

#include "backends/plugins/psp/psp-provider.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/osys_psp.h"
#include "backends/platform/psp/tests.h"	/* for unit/speed tests */
#include "backends/platform/psp/trace.h"

#ifdef ENABLE_PROFILING
	#include <pspprof.h>
#endif

/**
 * Define the module info section
 *
 * 2nd arg must 0x1000 so __init is executed in
 * kernelmode for our loaderInit function
 */
#ifndef USERSPACE_ONLY
PSP_MODULE_INFO("SCUMMVM-PSP", 0x1000, 1, 1);
#else
PSP_MODULE_INFO("SCUMMVM-PSP", 0, 1, 1);
#endif

/**
 * THREAD_ATTR_USER causes the thread that the startup
 * code (crt0.c) starts this program in to be in usermode
 * even though the module was started in kernelmode
 */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-128);	//Leave 128kb for thread stacks, etc.


#ifndef USERSPACE_ONLY
void MyExceptionHandler(PspDebugRegBlock *regs) {
	/* Do normal initial dump, setup screen etc */

	pspDebugScreenInit();

	pspDebugScreenSetBackColor(0x00FF0000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear();

	pspDebugScreenPrintf("Exception Details:\n");
	pspDebugDumpException(regs);

	while (1) ;
}

/**
 * Function that is called from _init in kernelmode before the
 * main thread is started in usermode.
 */
__attribute__((constructor))
void loaderInit() {
	pspKernelSetKernelPC();
	pspDebugInstallErrorHandler(MyExceptionHandler);
}
#endif

/* Exit callback */
int exit_callback(void) {

#ifdef ENABLE_PROFILING
	gprof_cleanup();
#endif

	sceKernelExitGame();
	return 0;
}

/* Function for handling suspend/resume */
int power_callback(int , int powerinfo, void *) {
	if (powerinfo & PSP_POWER_CB_POWER_SWITCH || powerinfo & PSP_POWER_CB_SUSPENDING) {
		PowerMan.suspend();
	} else if (powerinfo & PSP_POWER_CB_RESUME_COMPLETE) {
		PowerMan.resume();
	}
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize /*size*/, void *arg) {
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (SceKernelCallbackFunction)exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	/* Set up callbacks for PSPIoStream */

	cbid = sceKernelCreateCallback("Power Callback", (SceKernelCallbackFunction)power_callback, 0);
	if (cbid >= 0) {
		if (scePowerRegisterCallback(-1, cbid) < 0) {
			PSP_ERROR("Couldn't register callback for power_callback\n");
		}
	} else {
		PSP_ERROR("Couldn't create a callback for power_callback\n");
	}

	sceKernelSleepThreadCB();
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("power_thread", CallbackThread, PRIORITY_POWER_THREAD, STACK_POWER_THREAD, THREAD_ATTR_USER, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

#undef main
int main(void) {
	//change clock rate to 333mhz
	scePowerSetClockFrequency(333, 333, 166);

	PowerManager::instance();	// Setup power manager

	SetupCallbacks();

	static const char *argv[] = { "scummvm", NULL };
	static int argc = sizeof(argv) / sizeof(char *) - 1;

	g_system = new OSystem_PSP();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new PSPPluginProvider());
#endif

/* unit/speed tests */
#if defined(PSP_ENABLE_UNIT_TESTS) || defined(PSP_ENABLE_SPEED_TESTS)
	PSP_INFO_PRINT("running tests\n");
	psp_tests();
	sceKernelSleepThread();	// that's it. That's all we're doing
#endif

	int res = scummvm_main(argc, argv);

	g_system->quit();	// TODO: Consider removing / replacing this!

	PowerManager::destroy();	// get rid of PowerManager

	sceKernelSleepThread();

	return res;
}
