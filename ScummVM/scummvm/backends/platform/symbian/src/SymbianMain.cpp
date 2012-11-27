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

#include "backends/platform/symbian/src/portdefs.h"
#include "base/main.h"
#include "backends/platform/symbian/src/SymbianOS.h"

extern "C"
{
// Include the snprintf and vsnprintf implementations as 'C' code
#include "vsnprintf.h"
}

// Symbian SDL_Main implementation
// Redirects standard io, creates Symbian specific SDL backend (inherited from main SDL)
int main(int argc, char *argv[]) {
	//
	// Set up redirects for stdout/stderr under Symbian.
	// Code copied from SDL_main.
	//

	// Symbian does not like any output to the console through any *print* function
	char STDOUT_FILE[256], STDERR_FILE[256]; // shhh, don't tell anybody :)
	strcpy(STDOUT_FILE, Symbian::GetExecutablePath());
	strcpy(STDERR_FILE, Symbian::GetExecutablePath());
	strcat(STDOUT_FILE, "scummvm.stdout.txt");
	strcat(STDERR_FILE, "scummvm.stderr.txt");

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

	/* Redirect standard input and standard output */
	FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(STDOUT_FILE, "w");
#else
		newfp = fopen(STDOUT_FILE, "w");
		if (newfp) {
			*stdout = *newfp;
		}
#endif
	}
	newfp = freopen(STDERR_FILE, "w", stderr);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(STDERR_FILE, "w");
#else
		newfp = fopen(STDERR_FILE, "w");
		if (newfp) {
			*stderr = *newfp;
		}
#endif
	}
	setbuf(stderr, NULL);			/* No buffering */

	// Create our OSystem instance
	g_system = new OSystem_SDL_Symbian();
	assert(g_system);

	// Pre initialize the backend
	((OSystem_SDL_Symbian *)g_system)->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	delete (OSystem_SDL_Symbian *)g_system;

	return res;
}
