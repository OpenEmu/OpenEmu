/* 7zIn.h -- 7z Input functions
2008-11-23 : Igor Pavlov : Public domain */

#ifndef __7Z_IN_H
#define __7Z_IN_H

#include "7zHeader.h"
#include "7zItem.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct
{
  CSzAr db;
  
  UInt64 startPosAfterHeader;
  UInt64 dataPos;

  UInt32 *FolderStartPackStreamIndex;
  UInt64 *PackStreamStartPositions;
  UInt32 *FolderStartFileIndex;
  UInt32 *FileIndexToFolderIndexMap;
} CSzArEx;

void SzArEx_Init(CSzArEx *p);
void SzArEx_Free(CSzArEx *p, ISzAlloc *alloc);
UInt64 SzArEx_GetFolderStreamPos(const CSzArEx *p, UInt32 folderIndex, UInt32 indexInFolder);
int SzArEx_GetFolderFullPackSize(const CSzArEx *p, UInt32 folderIndex, UInt64 *resSize);

/*
Errors:
SZ_ERROR_NO_ARCHIVE
SZ_ERROR_ARCHIVE
SZ_ERROR_UNSUPPORTED
SZ_ERROR_MEM
SZ_ERROR_CRC
SZ_ERROR_INPUT_EOF
SZ_ERROR_FAIL
*/

SRes SzArEx_Open(CSzArEx *p, ILookInStream *inStream, ISzAlloc *allocMain, ISzAlloc *allocTemp);
 
#ifdef __cplusplus
	}
#endif

#endif
