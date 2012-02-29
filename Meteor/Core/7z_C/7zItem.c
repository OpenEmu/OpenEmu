/* 7zItem.c -- 7z Items
2008-10-04 : Igor Pavlov : Public domain */

#include "7zItem.h"

void SzCoderInfo_Init(CSzCoderInfo *p)
{
  Buf_Init(&p->Props);
}

void SzCoderInfo_Free(CSzCoderInfo *p, ISzAlloc *alloc)
{
  Buf_Free(&p->Props, alloc);
  SzCoderInfo_Init(p);
}

void SzFolder_Init(CSzFolder *p)
{
  p->Coders = 0;
  p->BindPairs = 0;
  p->PackStreams = 0;
  p->UnpackSizes = 0;
  p->NumCoders = 0;
  p->NumBindPairs = 0;
  p->NumPackStreams = 0;
  p->UnpackCRCDefined = 0;
  p->UnpackCRC = 0;
  p->NumUnpackStreams = 0;
}

static
void SzFolder_Free(CSzFolder *p, ISzAlloc *alloc)
{
  UInt32 i;
  if (p->Coders)
    for (i = 0; i < p->NumCoders; i++)
      SzCoderInfo_Free(&p->Coders[i], alloc);
  IAlloc_Free(alloc, p->Coders);
  IAlloc_Free(alloc, p->BindPairs);
  IAlloc_Free(alloc, p->PackStreams);
  IAlloc_Free(alloc, p->UnpackSizes);
  SzFolder_Init(p);
}

UInt32 SzFolder_GetNumOutStreams(CSzFolder *p)
{
  UInt32 result = 0;
  UInt32 i;
  for (i = 0; i < p->NumCoders; i++)
    result += p->Coders[i].NumOutStreams;
  return result;
}

int SzFolder_FindBindPairForInStream(CSzFolder *p, UInt32 inStreamIndex)
{
  UInt32 i;
  for (i = 0; i < p->NumBindPairs; i++)
    if (p->BindPairs[i].InIndex == inStreamIndex)
      return i;
  return -1;
}


static
int SzFolder_FindBindPairForOutStream(CSzFolder *p, UInt32 outStreamIndex)
{
  UInt32 i;
  for (i = 0; i < p->NumBindPairs; i++)
    if (p->BindPairs[i].OutIndex == outStreamIndex)
      return i;
  return -1;
}

UInt64 SzFolder_GetUnpackSize(CSzFolder *p)
{
  int i = (int)SzFolder_GetNumOutStreams(p);
  if (i == 0)
    return 0;
  for (i--; i >= 0; i--)
    if (SzFolder_FindBindPairForOutStream(p, i) < 0)
      return p->UnpackSizes[i];
  /* throw 1; */
  return 0;
}

void SzFile_Init(CSzFileItem *p)
{
  p->HasStream = 1;
  p->IsDir = 0;
  p->IsAnti = 0;
  p->FileCRCDefined = 0;
  p->MTimeDefined = 0;
  p->Name = 0;
}

static void SzFile_Free(CSzFileItem *p, ISzAlloc *alloc)
{
  IAlloc_Free(alloc, p->Name);
  SzFile_Init(p);
}

void SzAr_Init(CSzAr *p)
{
  p->PackSizes = 0;
  p->PackCRCsDefined = 0;
  p->PackCRCs = 0;
  p->Folders = 0;
  p->Files = 0;
  p->NumPackStreams = 0;
  p->NumFolders = 0;
  p->NumFiles = 0;
}

void SzAr_Free(CSzAr *p, ISzAlloc *alloc)
{
  UInt32 i;
  if (p->Folders)
    for (i = 0; i < p->NumFolders; i++)
      SzFolder_Free(&p->Folders[i], alloc);
  if (p->Files)
    for (i = 0; i < p->NumFiles; i++)
      SzFile_Free(&p->Files[i], alloc);
  IAlloc_Free(alloc, p->PackSizes);
  IAlloc_Free(alloc, p->PackCRCsDefined);
  IAlloc_Free(alloc, p->PackCRCs);
  IAlloc_Free(alloc, p->Folders);
  IAlloc_Free(alloc, p->Files);
  SzAr_Init(p);
}
