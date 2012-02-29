/* 7zCrc.c -- CRC32 calculation
2009-11-23 : Igor Pavlov : Public domain */

#include "7zCrc.h"
#include "CpuArch.h"

#define kCrcPoly 0xEDB88320

#ifdef MY_CPU_LE
#define CRC_NUM_TABLES 8
#else
#define CRC_NUM_TABLES 1
#endif

typedef UInt32 (MY_FAST_CALL *CRC_FUNC)(UInt32 v, const void *data, size_t size, const UInt32 *table);

static CRC_FUNC g_CrcUpdate;
UInt32 g_CrcTable[256 * CRC_NUM_TABLES];

#if CRC_NUM_TABLES == 1

#define CRC_UPDATE_BYTE_2(crc, b) (table[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

static UInt32 MY_FAST_CALL CrcUpdateT1(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  for (; size > 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  return v;
}

#else

UInt32 MY_FAST_CALL CrcUpdateT4(UInt32 v, const void *data, size_t size, const UInt32 *table);
UInt32 MY_FAST_CALL CrcUpdateT8(UInt32 v, const void *data, size_t size, const UInt32 *table);

#endif

UInt32 MY_FAST_CALL CrcUpdate(UInt32 v, const void *data, size_t size)
{
  return g_CrcUpdate(v, data, size, g_CrcTable);
}

UInt32 MY_FAST_CALL CrcCalc(const void *data, size_t size)
{
  return g_CrcUpdate(CRC_INIT_VAL, data, size, g_CrcTable) ^ CRC_INIT_VAL;
}

void MY_FAST_CALL CrcGenerateTable()
{
  UInt32 i;
  for (i = 0; i < 256; i++)
  {
    UInt32 r = i;
    unsigned j;
    for (j = 0; j < 8; j++)
      r = (r >> 1) ^ (kCrcPoly & ~((r & 1) - 1));
    g_CrcTable[i] = r;
  }
  #if CRC_NUM_TABLES == 1
  g_CrcUpdate = CrcUpdateT1;
  #else
  for (; i < 256 * CRC_NUM_TABLES; i++)
  {
    UInt32 r = g_CrcTable[i - 256];
    g_CrcTable[i] = g_CrcTable[r & 0xFF] ^ (r >> 8);
  }
  g_CrcUpdate = CrcUpdateT4;
  #ifdef MY_CPU_X86_OR_AMD64
  if (!CPU_Is_InOrder())
    g_CrcUpdate = CrcUpdateT8;
  #endif
  #endif
}
