
#ifndef _OSD_H_
#define _OSD_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <conio.h>

#include <SDL.h>
#include <stdlib.h>

#include "shared.h"
#include "main.h"
#include "config.h"
#include "error.h"
#include "unzip.h"
#include "fileio.h"

#define osd_input_Update sdl_input_update

#define GG_ROM    "./ggenie.bin"
#define AR_ROM    "./areplay.bin"
#define OS_ROM    "./bios.bin"
#define SK_ROM    "./sk.bin"
#define SK_UPMEM  "./sk2chip.bin"

#endif /* _OSD_H_ */
