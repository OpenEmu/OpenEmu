#ifndef _MDFN_DRIVERH
#define _MDFN_DRIVERH

#include <stdio.h>

#include "types.h"
#include "git.h"
#include "settings-driver.h"
#include "nes/debug.h"
#include "nes/ppu/palette-driver.h"
#include "mednafen-driver.h"
#include "netplay-driver.h"
#include "nes/sound-driver.h"
#include "state-driver.h"
#include "movie-driver.h"
#include "mempatcher-driver.h"
#include "nes/vsuni-driver.h"
#include "video-driver.h"

uint32 MDFND_GetTime(void);
int MDFNI_BeginWaveRecord(uint32 rate, int channels, char *fn);
int MDFNI_EndWaveRecord(void);
#endif
