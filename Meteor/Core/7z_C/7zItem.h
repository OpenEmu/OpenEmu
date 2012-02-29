/* 7zItem.h -- 7z Items
2008-10-04 : Igor Pavlov : Public domain */

#ifndef __7Z_ITEM_H
#define __7Z_ITEM_H

#include "7zBuf.h"

typedef struct
{
  UInt32 NumInStreams;
  UInt32 NumOutStreams;
  UInt64 MethodID;
  CBuf Props;
} CSzCoderInfo;

void SzCoderInfo_Init(CSzCoderInfo *p);
void SzCoderInfo_Free(CSzCoderInfo *p, ISzAlloc *alloc);

typedef struct
{
  UInt32 InIndex;
  UInt32 OutIndex;
} CBindPair;

typedef struct
{
  CSzCoderInfo *Coders;
  CBindPair *BindPairs;
  UInt32 *PackStreams;
  UInt64 *UnpackSizes;
  UInt32 NumCoders;
  UInt32 NumBindPairs;
  UInt32 NumPackStreams;
  int UnpackCRCDefined;
  UInt32 UnpackCRC;

  UInt32 NumUnpackStreams;
} CSzFolder;

void SzFolder_Init(CSzFolder *p);
UInt64 SzFolder_GetUnpackSize(CSzFolder *p);
int SzFolder_FindBindPairForInStream(CSzFolder *p, UInt32 inStreamIndex);
UInt32 SzFolder_GetNumOutStreams(CSzFolder *p);

typedef struct
{
  UInt32 Low;
  UInt32 High;
} CNtfsFileTime;

typedef struct
{
  CNtfsFileTime MTime;
  UInt64 Size;
  char *Name;
  UInt32 FileCRC;

  Byte HasStream;
  Byte IsDir;
  Byte IsAnti;
  Byte FileCRCDefined;
  Byte MTimeDefined;
} CSzFileItem;

void SzFile_Init(CSzFileItem *p);

typedef struct
{
  UInt64 *PackSizes;
  Byte *PackCRCsDefined;
  UInt32 *PackCRCs;
  CSzFolder *Folders;
  CSzFileItem *Files;
  UInt32 NumPackStreams;
  UInt32 NumFolders;
  UInt32 NumFiles;
} CSzAr;

void SzAr_Init(CSzAr *p);
void SzAr_Free(CSzAr *p, ISzAlloc *alloc);

#endif
