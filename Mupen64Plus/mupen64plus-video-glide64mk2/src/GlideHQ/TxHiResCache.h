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

#ifndef __TXHIRESCACHE_H__
#define __TXHIRESCACHE_H__

/* support hires textures
 *   0: disable
 *   1: enable
 */
#define HIRES_TEXTURE 1

#include "TxCache.h"
#include "TxQuantize.h"
#include "TxImage.h"
#include "TxReSample.h"
#include <boost/filesystem.hpp>

class TxHiResCache : public TxCache
{
private:
  int _maxwidth;
  int _maxheight;
  int _maxbpp;
  boolean _haveCache;
  boolean _abortLoad;
  TxImage *_txImage;
  TxQuantize *_txQuantize;
  TxReSample *_txReSample;
  boolean loadHiResTextures(boost::filesystem::wpath dir_path, boolean replace);
public:
  ~TxHiResCache();
  TxHiResCache(int maxwidth, int maxheight, int maxbpp, int options,
               const wchar_t *datapath, const wchar_t *cachepath,
               const wchar_t *ident, dispInfoFuncExt callback);
  boolean empty();
  boolean load(boolean replace);
};

#endif /* __TXHIRESCACHE_H__ */
