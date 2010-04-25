/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - osal/preproc.h                                     *
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
                       
/* this header file is for system-dependent #defines, #includes, and typedefs */

#if !defined (OSAL_PREPROC_H)
#define OSAL_PREPROC_H

#if defined(WIN32)
  #define osal_insensitive_strcmp(x, y) _stricmp(x, y)
  #define snprintf _snprintf
  #define strdup _strdup

  // for isnan()
  #include <float.h>
  #define isnan _isnan

  #define OSAL_BREAKPOINT_INTERRUPT __asm{ int 3 };
  #define ALIGN(BYTES,DATA) __declspec(align(BYTES)) DATA;
#else  /* Not WIN32 */
  #define osal_insensitive_strcmp(x, y) strcasecmp(x, y)

  #define OSAL_BREAKPOINT_INTERRUPT asm(" int $3; ");
  #define ALIGN(BYTES,DATA) DATA __attribute__((aligned(BYTES)));
#endif

#endif /* OSAL_PREPROC_H */

