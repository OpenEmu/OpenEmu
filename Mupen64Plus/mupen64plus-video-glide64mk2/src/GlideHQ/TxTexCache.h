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

#ifndef __TXTEXCACHE_H__
#define __TXTEXCACHE_H__

#include "TxCache.h"

class TxTexCache : public TxCache
{
public:
  ~TxTexCache();
  TxTexCache(int options, int cachesize, const wchar_t *path, const wchar_t *ident,
             dispInfoFuncExt callback);
  boolean add(uint64 checksum, /* checksum hi:palette low:texture */
              GHQTexInfo *info);
};

#endif /* __TXTEXCACHE_H__ */
