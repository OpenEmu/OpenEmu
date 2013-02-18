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

#include "TxFilter.h"

TxFilter *txFilter = NULL;

#ifdef __cplusplus
extern "C"{
#endif

TAPI boolean TAPIENTRY
txfilter_init(int maxwidth, int maxheight, int maxbpp, int options, int cachesize,
              wchar_t *datapath, wchar_t *cachepath, wchar_t*ident,
              dispInfoFuncExt callback)
{
  if (txFilter) return 0;

  txFilter = new TxFilter(maxwidth, maxheight, maxbpp, options, cachesize,
                           datapath, cachepath, ident, callback);

  return (txFilter ? 1 : 0);
}

TAPI void TAPIENTRY
txfilter_shutdown(void)
{
  if (txFilter) delete txFilter;

  txFilter = NULL;
}

TAPI boolean TAPIENTRY
txfilter(uint8 *src, int srcwidth, int srcheight, uint16 srcformat,
         uint64 g64crc, GHQTexInfo *info)
{
  if (txFilter)
    return txFilter->filter(src, srcwidth, srcheight, srcformat,
                               g64crc, info);

  return 0;
}

TAPI boolean TAPIENTRY
txfilter_hirestex(uint64 g64crc, uint64 r_crc64, uint16 *palette, GHQTexInfo *info)
{
  if (txFilter)
    return txFilter->hirestex(g64crc, r_crc64, palette, info);

  return 0;
}

TAPI uint64 TAPIENTRY
txfilter_checksum(uint8 *src, int width, int height, int size, int rowStride, uint8 *palette)
{
  if (txFilter)
    return txFilter->checksum64(src, width, height, size, rowStride, palette);

  return 0;
}

TAPI boolean TAPIENTRY
txfilter_dmptx(uint8 *src, int width, int height, int rowStridePixel, uint16 gfmt, uint16 n64fmt, uint64 r_crc64)
{
  if (txFilter)
    return txFilter->dmptx(src, width, height, rowStridePixel, gfmt, n64fmt, r_crc64);

  return 0;
}

TAPI boolean TAPIENTRY
txfilter_reloadhirestex()
{
  if (txFilter)
    return txFilter->reloadhirestex();

  return 0;
}

#ifdef __cplusplus
}
#endif
