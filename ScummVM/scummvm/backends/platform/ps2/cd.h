/*********************************************************************
 * Copyright (C) 2003 Tord Lindstrom (pukko@home.se)
 * This file is subject to the terms and conditions of the PS2Link License.
 * See the file LICENSE in the main directory of this distribution for more
 * details.
 */

#ifndef _CD_H_
#define _CD_H_

#define CDVD_INIT_WAIT   0
#define CDVD_INIT_NOWAIT 1
#define CDVD_EXIT        5

#ifdef __cplusplus
extern "C" {
#endif

int cdvdInit(int mode);
void cdvdExit(void);

#ifdef __cplusplus
}
#endif

#endif
