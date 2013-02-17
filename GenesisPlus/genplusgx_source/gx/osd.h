/*************************************************/
/* port specific stuff should be put there       */
/*************************************************/

#ifndef _OSD_H_
#define _OSD_H_

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>
#include <unistd.h>
#include <asndlib.h>
#include <oggplayer.h>
#include <zlib.h>

#ifdef HW_RVL
#include <di/di.h>
#include <wiiuse/wpad.h>
#include "vi_encoder.h"
#endif

#include "gx_input.h"
#include "gx_audio.h"
#include "gx_video.h"
#include "file_load.h"
#include "cheats.h"

#include "config.h"
#include "fileio.h"

#define DEFAULT_PATH  "/genplus"

/*************************************************/
/* required by Genesis Plus GX core              */
/*************************************************/
#define GG_ROM        "/genplus/lock-on/ggenie.bin"
#define AR_ROM        "/genplus/lock-on/areplay.bin"
#define SK_ROM        "/genplus/lock-on/sk.bin"
#define SK_UPMEM      "/genplus/lock-on/sk2chip.bin"
#define MS_BIOS_US    "/genplus/bios/bios_U.sms"
#define MS_BIOS_EU    "/genplus/bios/bios_E.sms"
#define MS_BIOS_JP    "/genplus/bios/bios_J.sms"
#define GG_BIOS       "/genplus/bios/bios.gg"
#define MD_BIOS       "/genplus/bios/bios_MD.bin"
#define CD_BIOS_US    "/genplus/bios/bios_CD_U.bin"
#define CD_BIOS_EU    "/genplus/bios/bios_CD_E.bin"
#define CD_BIOS_JP    "/genplus/bios/bios_CD_J.bin"
#define CD_BRAM_US    "/genplus/saves/cd/scd_U.brm"
#define CD_BRAM_EU    "/genplus/saves/cd/scd_E.brm"
#define CD_BRAM_JP    "/genplus/saves/cd/scd_J.brm"
#define CART_BRAM     "/genplus/saves/cd/cart.brm"

/*********************************************************/
/* implemented by Genesis Plus GX core (GC/Wii specific) */
/*********************************************************/

/* 32 bytes aligned sound buffers (8 samples alignment) */
#define ALIGN_SND 0xfffffff8

/* use Wii DVD LED to indicate when virtual CD tray is open */
#ifdef HW_RVL
#define CD_TRAY_CALLBACK *(u32*)0xcd0000c0 = (*(u32*)0xcd0000c0 & ~0x20) | ((cdd.status == CD_OPEN) << 5);
#endif

/*************************************************/

#define VERSION "Genesis Plus GX 1.7.3"

#define NO_SYNC    0
#define SYNC_VIDEO 1
#define SYNC_AUDIO 2

/* globals */
extern void legal(void);
extern double get_framerate(void);
extern void reloadrom(void);
extern void shutdown(void);
extern u32 Shutdown;
extern u32 ConfigRequested;
extern char osd_version[32];

#ifdef LOG_TIMING
#include <ogc/lwp_watchdog.h>
#define LOGSIZE 2000
extern u64 prevtime;
extern u32 frame_cnt;
extern u32 delta_time[LOGSIZE];
extern u32 delta_samp[LOGSIZE];
#endif

#endif /* _OSD_H_ */
