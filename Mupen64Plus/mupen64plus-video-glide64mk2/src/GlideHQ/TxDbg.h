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

#ifndef __TXDBG_H__
#define __TXDBG_H__

#include <stdio.h>
#include "TxInternal.h"

class TxDbg
{
private:
  FILE* _dbgfile;
  int _level;
  TxDbg();
public:
  static TxDbg* getInstance() {
    static TxDbg txDbg;
    return &txDbg;
  }
  ~TxDbg();
  void output(const int level, const wchar_t *format, ...);
};

#ifdef DEBUG
#define DBG_INFO(...) TxDbg::getInstance()->output(__VA_ARGS__)
#define INFO(...) DBG_INFO(__VA_ARGS__)
#else
#define DBG_INFO(...)
#ifdef GHQCHK
#define INFO(...) TxDbg::getInstance()->output(__VA_ARGS__)
#else
#if 0 /* XXX enable this to log basic hires texture checks */
#define INFO(...) TxDbg::getInstance()->output(__VA_ARGS__)
#else
#define INFO(...) DBG_INFO(__VA_ARGS__)
#endif
#endif
#endif

#endif /* __TXDBG_H__ */
