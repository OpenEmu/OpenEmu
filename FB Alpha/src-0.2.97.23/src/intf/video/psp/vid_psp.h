#ifndef __VIDGU_H__
#define __VIDGU_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <pspctrl.h>
#include <pspgu.h>

//extern unsigned short g_pBlitBuff[];
extern unsigned short *g_pBlitBuff;

void vidgu_init();

void vidgu_exit();

void vidgu_render_nostretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
void vidgu_render(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

