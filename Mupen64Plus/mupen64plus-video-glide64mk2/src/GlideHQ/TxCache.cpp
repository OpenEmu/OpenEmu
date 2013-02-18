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

#include "../Glide64/m64p.h"
#include "TxCache.h"
#include "TxDbg.h"
#include <zlib.h>
#include <boost/filesystem.hpp>

TxCache::~TxCache()
{
  /* free memory, clean up, etc */
  clear();

  delete _txUtil;
}

TxCache::TxCache(int options, int cachesize, const wchar_t *datapath,
                 const wchar_t *cachepath, const wchar_t *ident,
                 dispInfoFuncExt callback)
{
  _txUtil = new TxUtil();

  _options = options;
  _cacheSize = cachesize;
  _callback = callback;
  _totalSize = 0;

  /* save path name */
  if (datapath)
    _datapath.assign(datapath);
  if (cachepath)
    _cachepath.assign(cachepath);

  /* save ROM name */
  if (ident)
    _ident.assign(ident);

  /* zlib memory buffers to (de)compress hires textures */
  if (_options & (GZ_TEXCACHE|GZ_HIRESTEXCACHE)) {
    _gzdest0   = TxMemBuf::getInstance()->get(0);
    _gzdest1   = TxMemBuf::getInstance()->get(1);
    _gzdestLen = (TxMemBuf::getInstance()->size_of(0) < TxMemBuf::getInstance()->size_of(1)) ?
                  TxMemBuf::getInstance()->size_of(0) : TxMemBuf::getInstance()->size_of(1);

    if (!_gzdest0 || !_gzdest1 || !_gzdestLen) {
      _options &= ~(GZ_TEXCACHE|GZ_HIRESTEXCACHE);
      _gzdest0 = NULL;
      _gzdest1 = NULL;
      _gzdestLen = 0;
    }
  }
}

boolean
TxCache::add(uint64 checksum, GHQTexInfo *info, int dataSize)
{
  /* NOTE: dataSize must be provided if info->data is zlib compressed. */

  if (!checksum || !info->data) return 0;

  uint8 *dest = info->data;
  uint16 format = info->format;

  if (!dataSize) {
    dataSize = _txUtil->sizeofTx(info->width, info->height, info->format);

    if (!dataSize) return 0;

    if (_options & (GZ_TEXCACHE|GZ_HIRESTEXCACHE)) {
      /* zlib compress it. compression level:1 (best speed) */
      uLongf destLen = _gzdestLen;
      dest = (dest == _gzdest0) ? _gzdest1 : _gzdest0;
      if (compress2(dest, &destLen, info->data, dataSize, 1) != Z_OK) {
        dest = info->data;
        DBG_INFO(80, L"Error: zlib compression failed!\n");
      } else {
        DBG_INFO(80, L"zlib compressed: %.02fkb->%.02fkb\n", (float)dataSize/1000, (float)destLen/1000);
        dataSize = destLen;
        format |= GR_TEXFMT_GZ;
      }
    }
  }

  /* if cache size exceeds limit, remove old cache */
  if (_cacheSize > 0) {
    _totalSize += dataSize;
    if ((_totalSize > _cacheSize) && !_cachelist.empty()) {
      /* _cachelist is arranged so that frequently used textures are in the back */
      std::list<uint64>::iterator itList = _cachelist.begin();
      while (itList != _cachelist.end()) {
        /* find it in _cache */
        std::map<uint64, TXCACHE*>::iterator itMap = _cache.find(*itList);
        if (itMap != _cache.end()) {
          /* yep we have it. remove it. */
          _totalSize -= (*itMap).second->size;
          free((*itMap).second->info.data);
          delete (*itMap).second;
          _cache.erase(itMap);
        }
        itList++;

        /* check if memory cache has enough space */
        if (_totalSize <= _cacheSize)
          break;
      }
      /* remove from _cachelist */
      _cachelist.erase(_cachelist.begin(), itList);

      DBG_INFO(80, L"+++++++++\n");
    }
    _totalSize -= dataSize;
  }

  /* cache it */
  uint8 *tmpdata = (uint8*)malloc(dataSize);
  if (tmpdata) {
    TXCACHE *txCache = new TXCACHE;
    if (txCache) {
      /* we can directly write as we filter, but for now we get away
       * with doing memcpy after all the filtering is done.
       */
      memcpy(tmpdata, dest, dataSize);

      /* copy it */
      memcpy(&txCache->info, info, sizeof(GHQTexInfo));
      txCache->info.data = tmpdata;
      txCache->info.format = format;
      txCache->size = dataSize;

      /* add to cache */
      if (_cacheSize > 0) {
        _cachelist.push_back(checksum);
        txCache->it = --(_cachelist.end());
      }
      /* _cache[checksum] = txCache; */
      _cache.insert(std::map<uint64, TXCACHE*>::value_type(checksum, txCache));

#ifdef DEBUG
      DBG_INFO(80, L"[%5d] added!! crc:%08X %08X %d x %d gfmt:%x total:%.02fmb\n",
               _cache.size(), (uint32)(checksum >> 32), (uint32)(checksum & 0xffffffff),
               info->width, info->height, info->format, (float)_totalSize/1000000);

      DBG_INFO(80, L"smalllodlog2:%d largelodlog2:%d aspectratiolog2:%d\n",
               txCache->info.smallLodLog2, txCache->info.largeLodLog2, txCache->info.aspectRatioLog2);

      if (info->tiles) {
        DBG_INFO(80, L"tiles:%d un-tiled size:%d x %d\n", info->tiles, info->untiled_width, info->untiled_height);
      }

      if (_cacheSize > 0) {
        DBG_INFO(80, L"cache max config:%.02fmb\n", (float)_cacheSize/1000000);

        if (_cache.size() != _cachelist.size()) {
          DBG_INFO(80, L"Error: cache/cachelist mismatch! (%d/%d)\n", _cache.size(), _cachelist.size());
        }
      }
#endif

      /* total cache size */
      _totalSize += dataSize;

      return 1;
    }
    free(tmpdata);
  }

  return 0;
}

boolean
TxCache::get(uint64 checksum, GHQTexInfo *info)
{
  if (!checksum || _cache.empty()) return 0;

  /* find a match in cache */
  std::map<uint64, TXCACHE*>::iterator itMap = _cache.find(checksum);
  if (itMap != _cache.end()) {
    /* yep, we've got it. */
    memcpy(info, &(((*itMap).second)->info), sizeof(GHQTexInfo));

    /* push it to the back of the list */
    if (_cacheSize > 0) {
      _cachelist.erase(((*itMap).second)->it);
      _cachelist.push_back(checksum);
      ((*itMap).second)->it = --(_cachelist.end());
    }

    /* zlib decompress it */
    if (info->format & GR_TEXFMT_GZ) {
      uLongf destLen = _gzdestLen;
      uint8 *dest = (_gzdest0 == info->data) ? _gzdest1 : _gzdest0;
      if (uncompress(dest, &destLen, info->data, ((*itMap).second)->size) != Z_OK) {
        DBG_INFO(80, L"Error: zlib decompression failed!\n");
        return 0;
      }
      info->data = dest;
      info->format &= ~GR_TEXFMT_GZ;
      DBG_INFO(80, L"zlib decompressed: %.02fkb->%.02fkb\n", (float)(((*itMap).second)->size)/1000, (float)destLen/1000);
    }

    return 1;
  }

  return 0;
}

boolean
TxCache::save(const wchar_t *path, const wchar_t *filename, int config)
{
  if (!_cache.empty()) {
    /* dump cache to disk */
    char cbuf[MAX_PATH];

    boost::filesystem::wpath cachepath(path);
    boost::filesystem::create_directory(cachepath);

    /* Ugly hack to enable fopen/gzopen in Win9x */
#ifdef BOOST_WINDOWS_API
    wchar_t curpath[MAX_PATH];
    GETCWD(MAX_PATH, curpath);
    CHDIR(cachepath.wstring().c_str());
#else
    char curpath[MAX_PATH];
    wcstombs(cbuf, cachepath.wstring().c_str(), MAX_PATH);
    GETCWD(MAX_PATH, curpath);
    CHDIR(cbuf);
#endif

    wcstombs(cbuf, filename, MAX_PATH);

    gzFile gzfp = gzopen(cbuf, "wb1");
    DBG_INFO(80, L"gzfp:%x file:%ls\n", gzfp, filename);
    if (gzfp) {
      /* write header to determine config match */
      gzwrite(gzfp, &config, 4);

      std::map<uint64, TXCACHE*>::iterator itMap = _cache.begin();
      while (itMap != _cache.end()) {
        uint8 *dest    = (*itMap).second->info.data;
        uint32 destLen = (*itMap).second->size;
        uint16 format  = (*itMap).second->info.format;

        /* to keep things simple, we save the texture data in a zlib uncompressed state. */
        /* sigh... for those who cannot wait the extra few seconds. changed to keep
         * texture data in a zlib compressed state. if the GZ_TEXCACHE or GZ_HIRESTEXCACHE
         * option is toggled, the cache will need to be rebuilt.
         */
        /*if (format & GR_TEXFMT_GZ) {
          dest = _gzdest0;
          destLen = _gzdestLen;
          if (dest && destLen) {
            if (uncompress(dest, &destLen, (*itMap).second->info.data, (*itMap).second->size) != Z_OK) {
              dest = NULL;
              destLen = 0;
            }
            format &= ~GR_TEXFMT_GZ;
          }
        }*/

        if (dest && destLen) {
          /* texture checksum */
          gzwrite(gzfp, &((*itMap).first), 8);

          /* other texture info */
          gzwrite(gzfp, &((*itMap).second->info.width), 4);
          gzwrite(gzfp, &((*itMap).second->info.height), 4);
          gzwrite(gzfp, &format, 2);

          gzwrite(gzfp, &((*itMap).second->info.smallLodLog2), 4);
          gzwrite(gzfp, &((*itMap).second->info.largeLodLog2), 4);
          gzwrite(gzfp, &((*itMap).second->info.aspectRatioLog2), 4);

          gzwrite(gzfp, &((*itMap).second->info.tiles), 4);
          gzwrite(gzfp, &((*itMap).second->info.untiled_width), 4);
          gzwrite(gzfp, &((*itMap).second->info.untiled_height), 4);

          gzwrite(gzfp, &((*itMap).second->info.is_hires_tex), 1);

          gzwrite(gzfp, &destLen, 4);
          gzwrite(gzfp, dest, destLen);
        }

        itMap++;

        /* not ready yet */
        /*if (_callback)
          (*_callback)(L"Total textures saved to HDD: %d\n", std::distance(itMap, _cache.begin()));*/
      }
      gzclose(gzfp);
    }

    CHDIR(curpath);
  }

  return _cache.empty();
}

boolean
TxCache::load(const wchar_t *path, const wchar_t *filename, int config)
{
  /* find it on disk */
  char cbuf[MAX_PATH];

  boost::filesystem::wpath cachepath(path);

#ifdef BOOST_WINDOWS_API
  wchar_t curpath[MAX_PATH];
  GETCWD(MAX_PATH, curpath);
  CHDIR(cachepath.wstring().c_str());
#else
  char curpath[MAX_PATH];
  wcstombs(cbuf, cachepath.wstring().c_str(), MAX_PATH);
  GETCWD(MAX_PATH, curpath);
  CHDIR(cbuf);
#endif

  wcstombs(cbuf, filename, MAX_PATH);

  gzFile gzfp = gzopen(cbuf, "rb");
  DBG_INFO(80, L"gzfp:%x file:%ls\n", gzfp, filename);
  if (gzfp) {
    /* yep, we have it. load it into memory cache. */
    int dataSize;
    uint64 checksum;
    GHQTexInfo tmpInfo;
    int tmpconfig;
    /* read header to determine config match */
    gzread(gzfp, &tmpconfig, 4);

    if (tmpconfig == config) {
      do {
        memset(&tmpInfo, 0, sizeof(GHQTexInfo));

        gzread(gzfp, &checksum, 8);

        gzread(gzfp, &tmpInfo.width, 4);
        gzread(gzfp, &tmpInfo.height, 4);
        gzread(gzfp, &tmpInfo.format, 2);

        gzread(gzfp, &tmpInfo.smallLodLog2, 4);
        gzread(gzfp, &tmpInfo.largeLodLog2, 4);
        gzread(gzfp, &tmpInfo.aspectRatioLog2, 4);

        gzread(gzfp, &tmpInfo.tiles, 4);
        gzread(gzfp, &tmpInfo.untiled_width, 4);
        gzread(gzfp, &tmpInfo.untiled_height, 4);

        gzread(gzfp, &tmpInfo.is_hires_tex, 1);

        gzread(gzfp, &dataSize, 4);

        tmpInfo.data = (uint8*)malloc(dataSize);
        if (tmpInfo.data) {
          gzread(gzfp, tmpInfo.data, dataSize);

          /* add to memory cache */
          add(checksum, &tmpInfo, (tmpInfo.format & GR_TEXFMT_GZ) ? dataSize : 0);

          free(tmpInfo.data);
        } else {
          gzseek(gzfp, dataSize, SEEK_CUR);
        }

        /* skip in between to prevent the loop from being tied down to vsync */
        if (_callback && (!(_cache.size() % 100) || gzeof(gzfp)))
          (*_callback)(L"[%d] total mem:%.02fmb - %ls\n", _cache.size(), (float)_totalSize/1000000, filename);

      } while (!gzeof(gzfp));
      gzclose(gzfp);
    } else {
      if ((tmpconfig & HIRESTEXTURES_MASK) != (config & HIRESTEXTURES_MASK)) {
        const char *conf_str;
        if ((tmpconfig & HIRESTEXTURES_MASK) == NO_HIRESTEXTURES)
          conf_str = "0";
        else if ((tmpconfig & HIRESTEXTURES_MASK) == RICE_HIRESTEXTURES)
          conf_str = "1";
        else
          conf_str = "set to an unsupported format";

        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_hirs must be %s", conf_str);
      }
      if ((tmpconfig & COMPRESS_HIRESTEX) != (config & COMPRESS_HIRESTEX))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_hirs_cmpr must be %s", (tmpconfig & COMPRESS_HIRESTEX) ? "True" : "False");
      if ((tmpconfig & COMPRESSION_MASK) != (config & COMPRESSION_MASK) && (tmpconfig & COMPRESS_HIRESTEX)) {
        const char *conf_str;
        if ((tmpconfig & COMPRESSION_MASK) == FXT1_COMPRESSION)
          conf_str = "1";
        else if ((tmpconfig & COMPRESSION_MASK) == S3TC_COMPRESSION)
          conf_str = "0";
        else
          conf_str = "set to an unsupported format";

        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_cmpr must be %s", conf_str);
      }
      if ((tmpconfig & TILE_HIRESTEX) != (config & TILE_HIRESTEX))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_hirs_tile must be %s", (tmpconfig & TILE_HIRESTEX) ? "True" : "False");
      if ((tmpconfig & FORCE16BPP_HIRESTEX) != (config & FORCE16BPP_HIRESTEX))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_hirs_f16bpp must be %s", (tmpconfig & FORCE16BPP_HIRESTEX) ? "True" : "False");
      if ((tmpconfig & GZ_HIRESTEXCACHE) != (config & GZ_HIRESTEXCACHE))
        WriteLog(M64MSG_WARNING, "ghq_hirs_gz must be %s", (tmpconfig & GZ_HIRESTEXCACHE) ? "True" : "False");
      if ((tmpconfig & LET_TEXARTISTS_FLY) != (config & LET_TEXARTISTS_FLY))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_hirs_let_texartists_fly must be %s", (tmpconfig & LET_TEXARTISTS_FLY) ? "True" : "False");

      if ((tmpconfig & FILTER_MASK) != (config & FILTER_MASK)) {
        const char *conf_str;
        if ((tmpconfig & FILTER_MASK) == NO_FILTER)
          conf_str = "0";
        else if ((tmpconfig & FILTER_MASK) == SMOOTH_FILTER_1)
          conf_str = "1";
        else if ((tmpconfig & FILTER_MASK) == SMOOTH_FILTER_2)
          conf_str = "2";
        else if ((tmpconfig & FILTER_MASK) == SMOOTH_FILTER_3)
          conf_str = "3";
        else if ((tmpconfig & FILTER_MASK) == SMOOTH_FILTER_4)
          conf_str = "4";
        else if ((tmpconfig & FILTER_MASK) == SHARP_FILTER_1)
          conf_str = "5";
        else if ((tmpconfig & FILTER_MASK) == SHARP_FILTER_2)
          conf_str = "6";
        else
          conf_str = "set to an unsupported format";
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_fltr must be %s", conf_str);
      }

      if ((tmpconfig & ENHANCEMENT_MASK) != (config & ENHANCEMENT_MASK)) {
        const char *conf_str;
        if ((tmpconfig & ENHANCEMENT_MASK) == NO_ENHANCEMENT)
          conf_str = "0";
        else if ((tmpconfig & ENHANCEMENT_MASK) == X2_ENHANCEMENT)
          conf_str = "2";
        else if ((tmpconfig & ENHANCEMENT_MASK) == X2SAI_ENHANCEMENT)
          conf_str = "3";
        else if ((tmpconfig & ENHANCEMENT_MASK) == HQ2X_ENHANCEMENT)
          conf_str = "4";
        else if ((tmpconfig & ENHANCEMENT_MASK) == HQ2XS_ENHANCEMENT)
          conf_str = "5";
        else if ((tmpconfig & ENHANCEMENT_MASK) == LQ2X_ENHANCEMENT)
          conf_str = "6";
        else if ((tmpconfig & ENHANCEMENT_MASK) == LQ2XS_ENHANCEMENT)
          conf_str = "7";
        else if ((tmpconfig & ENHANCEMENT_MASK) == HQ4X_ENHANCEMENT)
          conf_str = "8";
        else
          conf_str = "set to an unsupported format";
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_enht must be %s", conf_str);
      }

      if ((tmpconfig & COMPRESS_TEX) != (config & COMPRESS_TEX))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_enht_cmpr must be %s", (tmpconfig & COMPRESS_TEX) ? "True" : "False");
      if ((tmpconfig & FORCE16BPP_TEX) != (config & FORCE16BPP_TEX))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_enht_f16bpp must be %s", (tmpconfig & FORCE16BPP_TEX) ? "True" : "False");
      if ((tmpconfig & GZ_TEXCACHE) != (config & GZ_TEXCACHE))
        WriteLog(M64MSG_WARNING, "Ignored texture cache due to incompatible setting: ghq_enht_gz must be %s", (tmpconfig & GZ_TEXCACHE) ? "True" : "False");
    }
  }

  CHDIR(curpath);

  return !_cache.empty();
}

boolean
TxCache::del(uint64 checksum)
{
  if (!checksum || _cache.empty()) return 0;

  std::map<uint64, TXCACHE*>::iterator itMap = _cache.find(checksum);
  if (itMap != _cache.end()) {

    /* for texture cache (not hi-res cache) */
    if (!_cachelist.empty()) _cachelist.erase(((*itMap).second)->it);

    /* remove from cache */
    free((*itMap).second->info.data);
    _totalSize -= (*itMap).second->size;
    delete (*itMap).second;
    _cache.erase(itMap);

    DBG_INFO(80, L"removed from cache: checksum = %08X %08X\n", (uint32)(checksum & 0xffffffff), (uint32)(checksum >> 32));

    return 1;
  }

  return 0;
}

boolean
TxCache::is_cached(uint64 checksum)
{
  std::map<uint64, TXCACHE*>::iterator itMap = _cache.find(checksum);
  if (itMap != _cache.end()) return 1;

  return 0;
}

void
TxCache::clear()
{
  if (!_cache.empty()) {
    std::map<uint64, TXCACHE*>::iterator itMap = _cache.begin();
    while (itMap != _cache.end()) {
      free((*itMap).second->info.data);
      delete (*itMap).second;
      itMap++;
    }
    _cache.clear();
  }

  if (!_cachelist.empty()) _cachelist.clear();

  _totalSize = 0;
}
