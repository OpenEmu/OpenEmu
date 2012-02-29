#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "Patch.h"


#ifdef __GNUC__
#if defined(__APPLE__) || defined (BSD) || defined (__NetBSD__)
typedef off_t __off64_t; /* off_t is 64 bits on BSD. */
#define	fseeko64 fseeko
#define	ftello64 ftello
#else
typedef off64_t __off64_t;
#endif /* __APPLE__ || BSD */
#endif /* __GNUC__ */

#ifndef _MSC_VER
#define _stricmp strcasecmp
#endif // ! _MSC_VER

#ifdef _MSC_VER
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
typedef	__int64 __off64_t;
#endif

static int readInt2(FILE *f)
{
  int res = 0;
  int c = fgetc(f);
  if(c == EOF)
    return -1;
  res = c;
  c = fgetc(f);
  if(c == EOF)
    return -1;
  return c + (res<<8);
}

static int readInt3(FILE *f)
{
  int res = 0;
  int c = fgetc(f);
  if(c == EOF)
    return -1;
  res = c;
  c = fgetc(f);
  if(c == EOF)
    return -1;
  res = c + (res<<8);
  c = fgetc(f);
  if(c == EOF)
    return -1;
  return c + (res<<8);
}

static s64 readInt4(FILE *f)
{
  s64 tmp, res = 0;
  int c;

  for (int i = 0; i < 4; i++) {
    c = fgetc(f);
    if (c == EOF)
      return -1;
    tmp = c;
    res = res + (tmp << (i*8));
  }

  return res;
}

static s64 readInt8(FILE *f)
{
  s64 tmp, res = 0;
  int c;

  for (int i = 0; i < 8; i++) {
    c = fgetc(f);
    if (c == EOF)
      return -1;
    tmp = c;
    res = res + (tmp << (i*8));
  }

  return res;
}

static s64 readVarPtr(FILE *f)
{
  s64 offset = 0, shift = 1;
  for (;;) {
    int c = fgetc(f);
    if (c == EOF) return 0;
    offset += (c & 0x7F) * shift;
    if (c & 0x80) break;
    shift <<= 7;
    offset += shift;
  }
  return offset;
}

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

static uLong computePatchCRC(FILE *f, unsigned int size)
{
  Bytef buf[4096];
  long readed;

  uLong crc = crc32(0L, Z_NULL, 0);
  do {
    readed = fread(buf, 1, MIN(size, sizeof(buf)), f);
    crc = crc32(crc, buf, readed);
    size -= readed;
  } while (readed > 0);
  return crc;
}

static bool patchApplyIPS(const char *patchname, u8 **r, int *s)
{
  // from the IPS spec at http://zerosoft.zophar.net/ips.htm
  FILE *f = fopen(patchname, "rb");
  if(!f)
    return false;

  bool result = false;

  u8 *rom = *r;
  int size = *s;
  if(fgetc(f) == 'P' &&
     fgetc(f) == 'A' &&
     fgetc(f) == 'T' &&
     fgetc(f) == 'C' &&
     fgetc(f) == 'H') {
    int b;
    int offset;
    int len;

    result = true;

    for(;;) {
      // read offset
      offset = readInt3(f);
      // if offset == EOF, end of patch
      if(offset == 0x454f46 || offset == -1)
        break;
      // read length
      len = readInt2(f);
      if(!len) {
        // len == 0, RLE block
        len = readInt2(f);
        // byte to fill
        int c = fgetc(f);
        if(c == -1)
          break;
        b = (u8)c;
      } else
        b= -1;
      // check if we need to reallocate our ROM
      if((offset + len) >= size) {
        size *= 2;
        rom = (u8 *)realloc(rom, size);
        *r = rom;
        *s = size;
      }
      if(b == -1) {
        // normal block, just read the data
        if(fread(&rom[offset], 1, len, f) != (size_t)len)
          break;
      } else {
        // fill the region with the given byte
        while(len--) {
          rom[offset++] = b;
        }
      }
    }
  }
  // close the file
  fclose(f);

  return result;
}

static bool patchApplyUPS(const char *patchname, u8 **rom, int *size)
{
  s64 srcCRC, dstCRC, patchCRC;

  FILE *f = fopen(patchname, "rb");
  if (!f)
    return false;

  fseeko64(f, 0, SEEK_END);
  __off64_t patchSize = ftello64(f);
  if (patchSize < 20) {
    fclose(f);
    return false;
  }

  fseeko64(f, 0, SEEK_SET);
  if(fgetc(f) != 'U' || fgetc(f) != 'P' || fgetc(f) != 'S' || fgetc(f) != '1') {
    fclose(f);
    return false;
  }

  fseeko64(f, -12, SEEK_END);
  srcCRC = readInt4(f);
  dstCRC = readInt4(f);
  patchCRC = readInt4(f);
  if (srcCRC == -1 || dstCRC == -1 || patchCRC == -1) {
    fclose(f);
    return false;
  }

  fseeko64(f, 0, SEEK_SET);
  u32 crc = computePatchCRC(f, patchSize-4);

  if (crc != patchCRC) {
    fclose(f);
    return false;
  }

  crc = crc32(0L, Z_NULL, 0);
  crc = crc32(crc, *rom, *size);

  fseeko64(f, 4, SEEK_SET);
  s64 dataSize;
  s64 srcSize = readVarPtr(f);
  s64 dstSize = readVarPtr(f);

  if (crc == srcCRC) {
    if (srcSize != *size) {
      fclose(f);
      return false;
    }
    dataSize = dstSize;
  } else if (crc == dstCRC) {
    if (dstSize != *size) {
      fclose(f);
      return false;
    }
    dataSize = srcSize;
  } else {
    fclose(f);
    return false;
  }
  if (dataSize > *size) {
    *rom = (u8*)realloc(*rom, dataSize);
    memset(*rom + *size, 0, dataSize - *size);
    *size = dataSize;
  }

  s64 relative = 0;
  u8 *mem;
  while(ftello64(f) < patchSize - 12) {
    relative += readVarPtr(f);
    if (relative > dataSize) continue;
    mem = *rom + relative;
    for(s64 i = relative; i < dataSize; i++) {
      int x = fgetc(f);
      relative++;
      if (!x) break;
      if (i < dataSize) {
        *mem++ ^= x;
      }
    }
  }

  fclose(f);
  return true;
}

static int ppfVersion(FILE *f)
{
  fseeko64(f, 0, SEEK_SET);
  if (fgetc(f) != 'P' || fgetc(f) != 'P' || fgetc(f) != 'F')
    return 0;
  switch(fgetc(f)){
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    default: return 0;
  }
}

static int ppfFileIdLen(FILE *f, int version)
{
  if (version == 2) {
    fseeko64(f, -8, SEEK_END);
  } else {
    fseeko64(f, -6, SEEK_END);
  }

  if (fgetc(f) != '.' || fgetc(f) != 'D' || fgetc(f) != 'I' || fgetc(f) != 'Z')
    return 0;

  return (version == 2) ? readInt4(f) : readInt2(f);
}

static bool patchApplyPPF1(FILE *f, u8 **rom, int *size)
{
  fseek(f, 0, SEEK_END);
  int count = ftell(f);
  if (count < 56)
    return false;
  count -= 56;

  fseek(f, 56, SEEK_SET);

  u8 *mem = *rom;

  while (count > 0) {
    int offset = readInt4(f);
    if (offset == -1)
      break;
    int len = fgetc(f);
    if (len == EOF)
      break;
    if (offset+len > *size)
      break;
    if (fread(&mem[offset], 1, len, f) != (size_t)len)
      break;
    count -= 4 + 1 + len;
  }

  return (count == 0);
}

static bool patchApplyPPF2(FILE *f, u8 **rom, int *size)
{
  fseek(f, 0, SEEK_END);
  int count = ftell(f);
  if (count < 56+4+1024)
    return false;
  count -= 56+4+1024;

  fseek(f, 56, SEEK_SET);

  int datalen = readInt4(f);
  if (datalen != *size)
    return false;

  u8 *mem = *rom;

  u8 block[1024];
  fread(&block, 1, 1024, f);
  if (memcmp(&mem[0x9320], &block, 1024) != 0)
    return false;

  int idlen = ppfFileIdLen(f, 2);
  if (idlen > 0)
    count -= 16 + 16 + idlen;

  fseek(f, 56+4+1024, SEEK_SET);

  while (count > 0) {
    int offset = readInt4(f);
    if (offset == -1)
      break;
    int len = fgetc(f);
    if (len == EOF)
      break;
    if (offset+len > *size)
      break;
    if (fread(&mem[offset], 1, len, f) != (size_t)len)
      break;
    count -= 4 + 1 + len;
  }

  return (count == 0);
}

static bool patchApplyPPF3(FILE *f, u8 **rom, int *size)
{
  fseek(f, 0, SEEK_END);
  int count = ftell(f);
  if (count < 56+4+1024)
    return false;
  count -= 56+4;

  fseek(f, 56, SEEK_SET);

  int imagetype = fgetc(f);
  int blockcheck = fgetc(f);
  int undo = fgetc(f);
  fgetc(f);

  u8 *mem = *rom;

  if (blockcheck) {
    u8 block[1024];
    fread(&block, 1, 1024, f);
    if (memcmp(&mem[(imagetype == 0) ? 0x9320 : 0x80A0], &block, 1024) != 0)
      return false;
    count -= 1024;
  }

  int idlen = ppfFileIdLen(f, 2);
  if (idlen > 0)
    count -= 16 + 16 + idlen;

  fseek(f, 56+4+(blockcheck ? 1024 : 0), SEEK_SET);

  while (count > 0) {
    __off64_t offset = readInt8(f);
    if (offset == -1)
      break;
    int len = fgetc(f);
    if (len == EOF)
      break;
    if (offset+len > *size)
      break;
    if (fread(&mem[offset], 1, len, f) != (size_t)len)
      break;
    if (undo) fseeko64(f, len, SEEK_CUR);
    count -= 8 + 1 + len;
    if (undo) count -= len;
  }

  return (count == 0);
}

static bool patchApplyPPF(const char *patchname, u8 **rom, int *size)
{
  FILE *f = fopen(patchname, "rb");
  if (!f)
    return false;

  bool res = false;

  int version = ppfVersion(f);
  switch (version) {
    case 1: res = patchApplyPPF1(f, rom, size); break;
    case 2: res = patchApplyPPF2(f, rom, size); break;
    case 3: res = patchApplyPPF3(f, rom, size); break;
  }

  fclose(f);
  return res;
}

bool applyPatch(const char *patchname, u8 **rom, int *size)
{
  if (strlen(patchname) < 5)
    return false;
  const char * p = strrchr(patchname, '.');
  if (p == NULL)
    return false;
  if (_stricmp(p, ".ips") == 0)
    return patchApplyIPS(patchname, rom, size);
  if (_stricmp(p, ".ups") == 0)
    return patchApplyUPS(patchname, rom, size);
  if (_stricmp(p, ".ppf") == 0)
    return patchApplyPPF(patchname, rom, size);
  return false;
}
