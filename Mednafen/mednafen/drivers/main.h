/* Mednafen - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _DRIVERS_MAIN_H
#define _DRIVERS_MAIN_H
#include "../driver.h"
#include "../mednafen.h"
#include "../settings.h"
#include "config.h"
#include "args.h"

#include <SDL.h>
#include <SDL_thread.h>

#include "../gettext.h"

#ifndef _
#define _(String) gettext(String)
#endif

#define CEVT_TOGGLEGUI	1
#define CEVT_TOGGLEFS	2
#define CEVT_VIDEOSYNC	5
#define CEVT_SHOWCURSOR		0x0c
#define CEVT_CHEATTOGGLEVIEW	0x10


#define CEVT_DISP_MESSAGE	0x11

#define CEVT_SET_GRAB_INPUT	0x20

#define CEVT_SET_STATE_STATUS	0x40
#define CEVT_SET_MOVIE_STATUS	0x41

#define CEVT_WANT_EXIT		0x80 // Emulator exit or GUI exit or bust!


#define CEVT_NP_TEXT_TO_SERVER	0x100
#define CEVT_NP_DISPLAY_TEXT	0x101
#define CEVT_NP_TOGGLE_VIEWABLE	0x102
#define CEVT_NP_TOGGLE_TT	0x103
#define CEVT_NP_CONNECT		0x104
#define CEVT_NP_SETNICK		0x105
#define CEVT_NP_PING		0x106
#define CEVT_NP_INTEGRITY	0x107

void SendCEvent(unsigned int code, void *data1, void *data2);

void PauseGameLoop(bool p);

void SDL_MDFN_ShowCursor(int toggle);

extern int NoWaiting;

extern MDFNGI *CurGame;
int CloseGame(void);

void RefreshThrottleFPS(double);
void PumpWrap(void);
void SetJoyReadMode(int mode);
void MainSetEventHook(int (*eh)(const SDL_Event *event));

void MainRequestExit(void);

extern bool pending_save_state, pending_snapshot, pending_save_movie;

void DoRunNormal(void);
void DoFrameAdvance(void);


void LockGameMutex(bool lock);
void DebuggerFudge(void);

extern volatile int GameThreadRun;

extern int sdlhaveogl;

bool MT_FromRemote_VideoSync(void);
bool MT_FromRemote_SoundSync(void);

void GT_ToggleFS(void);
void GT_ReinitVideo(void);

#endif
