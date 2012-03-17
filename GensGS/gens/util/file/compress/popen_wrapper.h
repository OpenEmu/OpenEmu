/*
 * popen_wrapper.h: Provides a popen()-like function for Win32.
 * Adapted from http://lists.trolltech.com/qt-interest/1999-09/thread00282-0.html
 */

#ifndef GENS_POPEN_WRAPPER_H
#define GENS_POPEN_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GENS_OS_WIN32

// Not Win32. Just use the POSIX version.
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#define gens_popen(command, type)	popen(command, type)
#define gens_pclose(handle)		pclose(handle)

#else /* GENS_OS_WIN32 */

// Win32. Use wrapper functions.
FILE* gens_popen(const char *cmd, const char *mode);
int gens_pclose(FILE *fle);

#endif /* GENS_OS_WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* GENS_POPEN_WRAPPER_H */
