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

#include "TxUtil.h"
#include "TxDbg.h"
#include <zlib.h>
#include <malloc.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
 * External libraries
 ******************************************************************************/
TxLoadLib::TxLoadLib()
{
#ifdef DXTN_DLL
  if (!_dxtnlib)
    _dxtnlib = LoadLibrary("dxtn");

  if (_dxtnlib) {
    if (!_tx_compress_dxtn)
      _tx_compress_dxtn = (dxtCompressTexFuncExt)DLSYM(_dxtnlib, "tx_compress_dxtn");

    if (!_tx_compress_fxt1)
      _tx_compress_fxt1 = (fxtCompressTexFuncExt)DLSYM(_dxtnlib, "fxt1_encode");
  }
#else
  _tx_compress_dxtn = tx_compress_dxtn;
  _tx_compress_fxt1 = fxt1_encode;

#endif
}

TxLoadLib::~TxLoadLib()
{
#ifdef DXTN_DLL
  /* free dynamic library */
  if (_dxtnlib)
    FreeLibrary(_dxtnlib);
#endif

}

fxtCompressTexFuncExt
TxLoadLib::getfxtCompressTexFuncExt()
{
  return _tx_compress_fxt1;
}

dxtCompressTexFuncExt
TxLoadLib::getdxtCompressTexFuncExt()
{
  return _tx_compress_dxtn;
}


/*
 * Utilities
 ******************************************************************************/
uint32
TxUtil::checksumTx(uint8 *src, int width, int height, uint16 format)
{
  int dataSize = sizeofTx(width, height, format);

  /* for now we use adler32 if something else is better
   * we can simply swtich later
   */
  /* return (dataSize ? Adler32(src, dataSize, 1) : 0); */

  /* zlib crc32 */
  return (dataSize ? crc32(crc32(0L, Z_NULL, 0), src, dataSize) : 0);
}

int
TxUtil::sizeofTx(int width, int height, uint16 format)
{
  int dataSize = 0;

  /* a lookup table for the shifts would be better */
  switch (format) {
  case GR_TEXFMT_ARGB_CMP_FXT1:
    dataSize = (((width + 0x7) & ~0x7) * ((height + 0x3) & ~0x3)) >> 1;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT1:
    dataSize = (((width + 0x3) & ~0x3) * ((height + 0x3) & ~0x3)) >> 1;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT3:
  case GR_TEXFMT_ARGB_CMP_DXT5:
    dataSize = ((width + 0x3) & ~0x3) * ((height + 0x3) & ~0x3);
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_44:
  case GR_TEXFMT_ALPHA_8:
  case GR_TEXFMT_INTENSITY_8:
  case GR_TEXFMT_P_8:
    dataSize = width * height;
    break;
  case GR_TEXFMT_ARGB_4444:
  case GR_TEXFMT_ARGB_1555:
  case GR_TEXFMT_RGB_565:
  case GR_TEXFMT_ALPHA_INTENSITY_88:
    dataSize = (width * height) << 1;
    break;
  case GR_TEXFMT_ARGB_8888:
    dataSize = (width * height) << 2;
    break;
  default:
    /* unsupported format */
    DBG_INFO(80, L"Error: cannot get size. unsupported gfmt:%x\n", format);
    ;
  }

  return dataSize;
}

#if 0 /* unused */
uint32
TxUtil::chkAlpha(uint32* src, int width, int height)
{
  /* NOTE: _src must be ARGB8888
   * return values
   * 0x00000000: 8bit alpha
   * 0x00000001: 1bit alpha
   * 0xff000001: no alpha
   */

  int _size = width * height;
  uint32 alpha = 0;

  __asm {
    mov esi, dword ptr [src];
    mov ecx, dword ptr [_size];
    mov ebx, 0xff000000;

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0xff000000;
    jz  alpha1bit;
    cmp eax, 0xff000000;
    je  alpha1bit;
    jmp done;

  alpha1bit:
    and ebx, eax;
    dec ecx;
    jnz tc1_loop;

    or  ebx, 0x00000001;
    mov dword ptr [alpha], ebx;

  done:
  }

  return alpha;
}
#endif

uint32
TxUtil::checksum(uint8 *src, int width, int height, int size, int rowStride)
{
  /* Rice CRC32 for now. We can switch this to Jabo MD5 or
   * any other custom checksum.
   * TODO: use *_HIRESTEXTURE option. */

  if (!src) return 0;

  return RiceCRC32(src, width, height, size, rowStride);
}

uint64
TxUtil::checksum64(uint8 *src, int width, int height, int size, int rowStride, uint8 *palette)
{
  /* Rice CRC32 for now. We can switch this to Jabo MD5 or
   * any other custom checksum.
   * TODO: use *_HIRESTEXTURE option. */
  /* Returned value is 64bits: hi=palette crc32 low=texture crc32 */

  if (!src) return 0;

  uint64 crc64Ret = 0;

  if (palette) {
    uint32 crc32 = 0, cimax = 0;
    switch (size & 0xff) {
    case 1:
      if (RiceCRC32_CI8(src, width, height, size, rowStride, &crc32, &cimax)) {
        crc64Ret = (uint64)RiceCRC32(palette, cimax + 1, 1, 2, 512);
        crc64Ret <<= 32;
        crc64Ret |= (uint64)crc32;
      }
      break;
    case 0:
      if (RiceCRC32_CI4(src, width, height, size, rowStride, &crc32, &cimax)) {
        crc64Ret = (uint64)RiceCRC32(palette, cimax + 1, 1, 2, 32);
        crc64Ret <<= 32;
        crc64Ret |= (uint64)crc32;
      }
    }
  }
  if (!crc64Ret) {
    crc64Ret = (uint64)RiceCRC32(src, width, height, size, rowStride);
  }

  return crc64Ret;
}

/*
** Computes Adler32 checksum for a stream of data.
**
** From the specification found in RFC 1950: (ZLIB Compressed Data Format
** Specification version 3.3)
**
** ADLER32 (Adler-32 checksum) This contains a checksum value of the
** uncompressed data (excluding any dictionary data) computed according to
** Adler-32 algorithm. This algorithm is a 32-bit extension and improvement
** of the Fletcher algorithm, used in the ITU-T X.224 / ISO 8073 standard.
**
** Adler-32 is composed of two sums accumulated per byte: s1 is the sum of
** all bytes, s2 is the sum of all s1 values. Both sums are done modulo
** 65521. s1 is initialized to 1, s2 to zero. The Adler-32 checksum is stored
** as s2*65536 + s1 in most-significant-byte first (network) order.
**
** 8.2. The Adler-32 algorithm 
**
** The Adler-32 algorithm is much faster than the CRC32 algorithm yet still
** provides an extremely low probability of undetected errors.
**
** The modulo on unsigned long accumulators can be delayed for 5552 bytes,
** so the modulo operation time is negligible. If the bytes are a, b, c,
** the second sum is 3a + 2b + c + 3, and so is position and order sensitive,
** unlike the first sum, which is just a checksum. That 65521 is prime is
** important to avoid a possible large class of two-byte errors that leave
** the check unchanged. (The Fletcher checksum uses 255, which is not prime
** and which also makes the Fletcher check insensitive to single byte
** changes 0 <-> 255.)
**
** The sum s1 is initialized to 1 instead of zero to make the length of
** the sequence part of s2, so that the length does not have to be checked
** separately. (Any sequence of zeroes has a Fletcher checksum of zero.)
*/

uint32
TxUtil::Adler32(const uint8* data, int Len, uint32 dwAdler32)
{
#if 1
  /* zlib adler32 */
  return adler32(dwAdler32, data, Len);
#else
  register uint32 s1 = dwAdler32 & 0xFFFF;
  register uint32 s2 = (dwAdler32 >> 16) & 0xFFFF;
  int k;

  while (Len > 0) {
    /* 5552 is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */
    k = (Len < 5552 ? Len : 5552);
    Len -= k;
    while (k--) {
      s1 += *data++;
      s2 += s1;
    }
    /* 65521 is the largest prime smaller than 65536 */
    s1 %= 65521;
    s2 %= 65521;
  }

  return (s2 << 16) | s1;
#endif
}

uint32
TxUtil::Adler32(const uint8* src, int width, int height, int size, int rowStride)
{
  int i;
  uint32 ret = 1;
  uint32 width_in_bytes = width * size;

  for (i = 0; i < height; i++) {
    ret = Adler32(src, width_in_bytes, ret);
    src += rowStride;
  }

  return ret;
}

// rotate left
template<class T> static T __ROL__(T value, unsigned int count)
{
  const unsigned int nbits = sizeof(T) * 8;
  count %= nbits;

  T high = value >> (nbits - count);
  value <<= count;
  value |= high;
  return value;
}

/* Rice CRC32 for hires texture packs */
/* NOTE: The following is used in Glide64 to calculate the CRC32
 * for Rice hires texture packs.
 *
 * BYTE* addr = (BYTE*)(gfx.RDRAM +
 *                     rdp.addr[rdp.tiles[tile].t_mem] +
 *                     (rdp.tiles[tile].ul_t * bpl) +
 *                     (((rdp.tiles[tile].ul_s<<rdp.tiles[tile].size)+1)>>1));
 * RiceCRC32(addr,
 *          rdp.tiles[tile].width,
 *          rdp.tiles[tile].height,
 *          (unsigned short)(rdp.tiles[tile].format << 8 | rdp.tiles[tile].size),
 *          bpl);
 */
uint32
TxUtil::RiceCRC32(const uint8* src, int width, int height, int size, int rowStride)
{
  const uint8_t *row;
  uint32_t crc32Ret;
  int cur_height;
  uint32_t pos;
  uint32_t word;
  uint32_t word_hash;
  uint32_t tmp;
  const uint32_t bytes_per_width = ((width << size) + 1) >> 1;

  row = src;
  crc32Ret = 0;

  for (cur_height = height - 1; cur_height >= 0; cur_height--) {
    for (pos = bytes_per_width - 4; pos < 0x80000000u; pos -= 4) {
      word = *(uint32_t *)&row[pos];
      word_hash = pos ^ word;
      tmp = __ROL__(crc32Ret, 4);
      crc32Ret = word_hash + tmp;
    }
    crc32Ret += cur_height ^ word_hash;
    row += rowStride;
  }
  return crc32Ret;
}

boolean
TxUtil::RiceCRC32_CI4(const uint8* src, int width, int height, int size, int rowStride,
                        uint32* crc32, uint32* cimax)
{
  const uint8_t *row;
  uint32_t crc32Ret;
  uint32_t cimaxRet;
  int cur_height;
  uint32_t pos;
  uint32_t word;
  uint32_t word_hash;
  uint32_t tmp;
  const uint32_t bytes_per_width = ((width << size) + 1) >> 1;

  row = src;
  crc32Ret = 0;
  cimaxRet = 0;

  for (cur_height = height - 1; cur_height >= 0; cur_height--) {
    for (pos = bytes_per_width - 4; pos < 0x80000000u; pos -= 4) {
      word = *(uint32_t *)&row[pos];
      if (cimaxRet != 15) {
        if ((word & 0xF) >= cimaxRet)
          cimaxRet = word & 0xF;
        if ((uint32_t)((uint8_t)word >> 4) >= cimaxRet)
          cimaxRet = (uint8_t)word >> 4;
        if (((word >> 8) & 0xF) >= cimaxRet)
          cimaxRet = (word >> 8) & 0xF;
        if ((uint32_t)((uint16_t)word >> 12) >= cimaxRet)
          cimaxRet = (uint16_t)word >> 12;
        if (((word >> 16) & 0xF) >= cimaxRet)
          cimaxRet = (word >> 16) & 0xF;
        if (((word >> 20) & 0xF) >= cimaxRet)
          cimaxRet = (word >> 20) & 0xF;
        if (((word >> 24) & 0xF) >= cimaxRet)
          cimaxRet = (word >> 24) & 0xF;
        if (word >> 28 >= cimaxRet )
          cimaxRet = word >> 28;
      }
      word_hash = pos ^ word;
      tmp = __ROL__(crc32Ret, 4);
      crc32Ret = word_hash + tmp;
    }
    crc32Ret += cur_height ^ word_hash;
    row += rowStride;
  }
  *crc32 = crc32Ret;
  *cimax = cimaxRet;
  return 1;
}

boolean
TxUtil::RiceCRC32_CI8(const uint8* src, int width, int height, int size, int rowStride,
                      uint32* crc32, uint32* cimax)
{
  const uint8_t *row;
  uint32_t crc32Ret;
  uint32_t cimaxRet;
  int cur_height;
  uint32_t pos;
  uint32_t word;
  uint32_t word_hash;
  uint32_t tmp;
  const uint32_t bytes_per_width = ((width << size) + 1) >> 1;

  row = src;
  crc32Ret = 0;
  cimaxRet = 0;

  for (cur_height = height - 1; cur_height >= 0; cur_height--) {
    for (pos = bytes_per_width - 4; pos < 0x80000000u; pos -= 4) {
      word = *(uint32_t *)&row[pos];
      if (cimaxRet != 255) {
        if ((uint8_t)word >= cimaxRet)
          cimaxRet = (uint8_t)word;
        if ((uint32_t)((uint16_t)word >> 8) >= cimaxRet)
          cimaxRet = (uint16_t)word >> 8;
        if (((word >> 16) & 0xFF) >= cimaxRet)
          cimaxRet = (word >> 16) & 0xFF;
        if (word >> 24 >= cimaxRet)
          cimaxRet = word >> 24;
      }
      word_hash = pos ^ word;
      tmp = __ROL__(crc32Ret, 4);
      crc32Ret = word_hash + tmp;
    }
    crc32Ret += cur_height ^ word_hash;
    row += rowStride;
  }
  *crc32 = crc32Ret;
  *cimax = cimaxRet;
  return 1;
}

int
TxUtil::log2(int num)
{
#if defined(__GNUC__)
  return __builtin_ctz(num);
#elif defined(_MSC_VER) && _MSC_VER >= 1400
  uint32_t i;
  _BitScanForward((DWORD *)&i, num);
  return i;
#elif defined(__MSC__)
  __asm {
    mov eax, dword ptr [num];
    bsr eax, eax;
    mov dword ptr [i], eax;
  }
#else
  switch (num) {
    case 1:    return 0;
    case 2:    return 1;
    case 4:    return 2;
    case 8:    return 3;
    case 16:   return 4;
    case 32:   return 5;
    case 64:   return 6;
    case 128:  return 7;
    case 256:  return 8;
    case 512:  return 9;
    case 1024:  return 10;
    case 2048:  return 11;
  }
#endif
}

int
TxUtil::grLodLog2(int w, int h)
{
  return (w >= h ? log2(w) : log2(h));
}

int
TxUtil::grAspectRatioLog2(int w, int h)
{
  return (w >= h ? log2(w/h) : -log2(h/w));
}

int
TxUtil::getNumberofProcessors()
{
  int numcore = 1, ret;

#ifdef _WIN32
#ifndef _SC_NPROCESSORS_ONLN
  SYSTEM_INFO info;
  GetSystemInfo(&info);
#define sysconf(a) info.dwNumberOfProcessors
#define _SC_NPROCESSORS_ONLN
#endif
#endif
#ifdef _SC_NPROCESSORS_ONLN
  ret = sysconf(_SC_NPROCESSORS_CONF);
  if (ret >= 1) {
    numcore = ret;
  }
  ret = sysconf(_SC_NPROCESSORS_ONLN);
  if (ret < 1) {
    numcore = ret;
  }
#endif

  return numcore;
}


/*
 * Memory buffers for texture manipulations
 ******************************************************************************/
TxMemBuf::TxMemBuf()
{
  int i;
  for (i = 0; i < 2; i++) {
    _tex[i] = NULL;
    _size[i] = 0;
  }
}

TxMemBuf::~TxMemBuf()
{
  shutdown();
}

boolean
TxMemBuf::init(int maxwidth, int maxheight)
{
  int i;
  for (i = 0; i < 2; i++) {
    if (!_tex[i]) {
      _tex[i] = (uint8 *)malloc(maxwidth * maxheight * 4);
      _size[i] = maxwidth * maxheight * 4;
    }

    if (!_tex[i]) {
      shutdown();
      return 0;
    }
  }
  return 1;
}

void
TxMemBuf::shutdown()
{
  int i;
  for (i = 0; i < 2; i++) {
    if (_tex[i]) free(_tex[i]);
    _tex[i] = NULL;
    _size[i] = 0;
  }
}

uint8*
TxMemBuf::get(unsigned int num)
{
  return ((num < 2) ? _tex[num] : NULL);
}

uint32
TxMemBuf::size_of(unsigned int num)
{
  return ((num < 2) ? _size[num] : 0);
}
