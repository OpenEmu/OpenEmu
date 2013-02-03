/*
 * Texture Filtering
 * Version:  1.0
 *
 * Copyright (C) 2007  Hiroshi Morii   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define DBG_LEVEL 80

#include "TxDbg.h"
#include <string.h>
#include <stdarg.h>
#include <boost/format.hpp>

TxDbg::TxDbg()
{
  _level = DBG_LEVEL;

  if (!_dbgfile)
#ifdef GHQCHK
    _dbgfile = fopen("ghqchk.txt", "w");
#else
    _dbgfile = fopen("glidehq.dbg", "w");
#endif
}

TxDbg::~TxDbg()
{
  if (_dbgfile) {
    fclose(_dbgfile);
    _dbgfile = 0;
  }

  _level = DBG_LEVEL;
}

void
TxDbg::output(const int level, const wchar_t *format, ...)
{
  va_list args;
  //wchar_t newformat[4095];
  std::wstring newformat;

  if (level > _level)
    return;

  va_start(args, format);
  //swprintf(newformat, 4095, L"%d:\t", level);
  //wcscat(newformat, format);
  //vfwprintf(_dbgfile, newformat, args);
  newformat = boost::str(boost::wformat(L"%d:\t%ls") % level % format);
  vfwprintf(_dbgfile, newformat.c_str(), args);
  fflush(_dbgfile);
#ifdef GHQCHK
  //vwprintf(newformat, args);
  vwprintf(newformat.c_str(), args);
#endif
  va_end(args);
}
