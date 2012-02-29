/* 7zCrcOpt.c -- CRC32 calculation : optimized version
2009-11-23 : Igor Pavlov : Public domain */

#include "CpuArch.h"

#ifdef MY_CPU_LE

#define CRC_UPDATE_BYTE_2(crc, b) (table[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

UInt32 MY_FAST_CALL CrcUpdateT4(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  for (; size > 0 && ((unsigned)(ptrdiff_t)p & 3) != 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  for (; size >= 4; size -= 4, p += 4)
  {
    v ^= *(const UInt32 *)p;
    v =
      table[0x300 + (v & 0xFF)] ^
      table[0x200 + ((v >> 8) & 0xFF)] ^
      table[0x100 + ((v >> 16) & 0xFF)] ^
      table[0x000 + ((v >> 24))];
  }
  for (; size > 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  return v;
}

UInt32 MY_FAST_CALL CrcUpdateT8(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  return CrcUpdateT4(v, data, size, table);
}

#endif
