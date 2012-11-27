/* Header is not present in Windows CE SDK */

#ifndef A800_TIME_H
#define A800_TIME_H

#include <stdlib.h>

#ifndef __MINGW32CE__
struct tm {
	short tm_year;
	short tm_mon;
	short tm_mday;
	short tm_wday;
	short tm_hour;
	short tm_min;
	short tm_sec;
};

#ifdef __GNUC__
#define EXT_C extern "C"
#else
#define EXT_C
#endif

EXT_C time_t time(time_t *dummy);
EXT_C struct tm *localtime(time_t *dummy);

unsigned int clock();

#endif
#endif
