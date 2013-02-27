/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*   Mupen64plus - version.h                                               *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
*   Copyright (C) 2008-2012 Richard42 DarkJeztr Tillin9                   *
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

/* Version macros automatically replaced by Makefiles. */

#ifndef __VERSION_H__
#define __VERSION_H__

#define MUPEN_CORE_NAME "Mupen64Plus Core"
#define MUPEN_CORE_VERSION 0x016305

#define FRONTEND_API_VERSION 0x020100
#define CONFIG_API_VERSION   0x020200
#define DEBUG_API_VERSION    0x020000
#define VIDEXT_API_VERSION   0x020000

#define VERSION_PRINTF_SPLIT(x) (((x) >> 16) & 0xffff), (((x) >> 8) & 0xff), ((x) & 0xff)

#endif /* __VERSION_H__ */


