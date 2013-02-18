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

#ifdef __MSC__
#pragma warning(disable: 4786)
#endif

/* dump cache to disk (0:disable, 1:enable) */
#define DUMP_CACHE 1

#include "TxTexCache.h"
#include "TxDbg.h"
#include <zlib.h>
#include <string>
#include <boost/filesystem.hpp>

TxTexCache::~TxTexCache()
{
#if DUMP_CACHE
  if (_options & DUMP_TEXCACHE) {
    /* dump cache to disk */
    std::wstring filename = _ident + L"_MEMORYCACHE.dat";
    boost::filesystem::wpath cachepath(_cachepath);
    cachepath /= boost::filesystem::wpath(L"glidehq");
    int config = _options & (FILTER_MASK|ENHANCEMENT_MASK|COMPRESS_TEX|COMPRESSION_MASK|FORCE16BPP_TEX|GZ_TEXCACHE);

    TxCache::save(cachepath.wstring().c_str(), filename.c_str(), config);
  }
#endif
}

TxTexCache::TxTexCache(int options, int cachesize, const wchar_t *datapath, const wchar_t *cachepath,
                       const wchar_t *ident, dispInfoFuncExt callback
                       ) : TxCache((options & ~GZ_HIRESTEXCACHE), cachesize, datapath, cachepath, ident, callback)
{
  /* assert local options */
  if (_cachepath.empty() || _ident.empty() || !_cacheSize)
    _options &= ~DUMP_TEXCACHE;

#if DUMP_CACHE
  if (_options & DUMP_TEXCACHE) {
    /* find it on disk */
    std::wstring filename = _ident + L"_MEMORYCACHE.dat";
    boost::filesystem::wpath cachepath(_cachepath);
    cachepath /= boost::filesystem::wpath(L"glidehq");
    int config = _options & (FILTER_MASK|ENHANCEMENT_MASK|COMPRESS_TEX|COMPRESSION_MASK|FORCE16BPP_TEX|GZ_TEXCACHE);

    TxCache::load(cachepath.wstring().c_str(), filename.c_str(), config);
  }
#endif
}

boolean
TxTexCache::add(uint64 checksum, GHQTexInfo *info)
{
  if (_cacheSize <= 0) return 0;

  return TxCache::add(checksum, info);
}
