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

#ifndef __TXCACHE_H__
#define __TXCACHE_H__

#include "TxInternal.h"
#include "TxUtil.h"
#include <list>
#include <map>
#include <string>

class TxCache
{
private:
  std::list<uint64> _cachelist;
  uint8 *_gzdest0;
  uint8 *_gzdest1;
  uint32 _gzdestLen;
protected:
  int _options;
  std::wstring _ident;
  std::wstring _datapath;
  std::wstring _cachepath;
  dispInfoFuncExt _callback;
  TxUtil *_txUtil;
  struct TXCACHE {
    int size;
    GHQTexInfo info;
    std::list<uint64>::iterator it;
  };
  int _totalSize;
  int _cacheSize;
  std::map<uint64, TXCACHE*> _cache;
  boolean save(const wchar_t *path, const wchar_t *filename, const int config);
  boolean load(const wchar_t *path, const wchar_t *filename, const int config);
  boolean del(uint64 checksum); /* checksum hi:palette low:texture */
  boolean is_cached(uint64 checksum); /* checksum hi:palette low:texture */
  void clear();
public:
  ~TxCache();
  TxCache(int options, int cachesize, const wchar_t *datapath,
              const wchar_t *cachepath, const wchar_t *ident,
              dispInfoFuncExt callback);
  boolean add(uint64 checksum, /* checksum hi:palette low:texture */
              GHQTexInfo *info, int dataSize = 0);
  boolean get(uint64 checksum, /* checksum hi:palette low:texture */
              GHQTexInfo *info);
};

#endif /* __TXCACHE_H__ */
