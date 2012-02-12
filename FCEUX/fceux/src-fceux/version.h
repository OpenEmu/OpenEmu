/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2001 Aaron Oneal
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

#ifndef __FCEU_VERSION
#define __FCEU_VERSION

//todo - everyone will want to support this eventually, i suppose
#ifdef _MSC_VER
#include "svnrev.h"
#else
#ifdef SVN_REV
#define SVN_REV_STR SVN_REV
#else
#define SVN_REV_STR ""
#endif
#endif

#define FCEU_NAME "FCEUX"

#define FCEU_FEATURE_STRING ""

#ifdef _DEBUG
#define FCEU_SUBVERSION_STRING " debug"
#elif defined(PUBLIC_RELEASE)
#define FCEU_SUBVERSION_STRING ""
#else
#define FCEU_SUBVERSION_STRING "-interim svn" SVN_REV_STR
#endif

#if defined(_MSC_VER)
#define FCEU_COMPILER ""
#define FCEU_COMPILER_DETAIL " msvc " _Py_STRINGIZE(_MSC_VER)
#define _Py_STRINGIZE(X) _Py_STRINGIZE1((X))
#define _Py_STRINGIZE1(X) _Py_STRINGIZE2 ## X
#define _Py_STRINGIZE2(X) #X
//re: http://72.14.203.104/search?q=cache:HG-okth5NGkJ:mail.python.org/pipermail/python-checkins/2002-November/030704.html+_msc_ver+compiler+version+string&hl=en&gl=us&ct=clnk&cd=5
#else
// TODO: make for others compilers
#define FCEU_COMPILER ""
#define FCEU_COMPILER_DETAIL ""
#endif

#define FCEU_VERSION_NUMERIC 21040
#define FCEU_VERSION_STRING "2.1.5" FCEU_SUBVERSION_STRING FCEU_FEATURE_STRING FCEU_COMPILER
#define FCEU_NAME_AND_VERSION FCEU_NAME " " FCEU_VERSION_STRING

#endif
