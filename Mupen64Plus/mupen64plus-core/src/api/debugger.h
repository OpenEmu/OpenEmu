/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - api/debugger.h                                     *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This file contains the definitions for debugger functions which will be
 * called from the other Core modules
 */

#if !defined(API_DEBUGGER_H)
#define API_DEBUGGER_H

#include "m64p_types.h"

/* Debugger Definitions */

typedef enum {
    DEBUG_UI_INIT = 1,
    DEBUG_UI_UPDATE,
    DEBUG_UI_VI,
    DEBUG_CORE_COMPARE
  } eDbgCallbackType;

/* Functions for use by the Core, to send information back to the front-end app */
extern void DebuggerCallback(eDbgCallbackType type, unsigned int param);
extern void CoreCompareCallback(void);
extern void CoreCompareDataSync(int length, void *ptr);

#endif /* API_DEBUGGER_H */

