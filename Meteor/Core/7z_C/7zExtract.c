/* 7zExtract.c -- Extracting from 7z archive
2008-11-23 : Igor Pavlov : Public domain */

#include "7zCrc.h"
#include "7zDecode.h"
#include "7zExtract.h"

SRes SzAr_Extract(
    const CSzArEx *p,
    ILookInStream *inStream,
    UInt32 fileIndex,
    UInt32 *blockIndex,
    Byte **outBuffer,
    size_t *outBufferSize,
    size_t *offset,
    size_t *outSizeProcessed,
    ISzAlloc *allocMain,
    ISzAlloc *allocTemp)
{
  UInt32 folderIndex = p->FileIndexToFolderIndexMap[fileIndex];
  SRes res = SZ_OK;
  *offset = 0;
  *outSizeProcessed = 0;
  if (folderIndex == (UInt32)-1)
  {
    IAlloc_Free(allocMain, *outBuffer);
    *blockIndex = folderIndex;
    *outBuffer = 0;
    *outBufferSize = 0;
    return SZ_OK;
  }

  if (*outBuffer == 0 || *blockIndex != folderIndex)
  {
    CSzFolder *folder = p->db.Folders + folderIndex;
    UInt64 unpackSizeSpec = SzFolder_GetUnpackSize(folder);
    size_t unpackSize = (size_t)unpackSizeSpec;
    UInt64 startOffset = SzArEx_GetFolderStreamPos(p, folderIndex, 0);

    if (unpackSize != unpackSizeSpec)
      return SZ_ERROR_MEM;
    *blockIndex = folderIndex;
    IAlloc_Free(allocMain, *outBuffer);
    *outBuffer = 0;
    
    RINOK(LookInStream_SeekTo(inStream, startOffset));
    
    if (res == SZ_OK)
    {
      *outBufferSize = unpackSize;
      if (unpackSize != 0)
      {
        *outBuffer = (Byte *)IAlloc_Alloc(allocMain, unpackSize);
        if (*outBuffer == 0)
          res = SZ_ERROR_MEM;
      }
      if (res == SZ_OK)
      {
        res = SzDecode(p->db.PackSizes +
          p->FolderStartPackStreamIndex[folderIndex], folder,
          inStream, startOffset,
          *outBuffer, unpackSize, allocTemp);
        if (res == SZ_OK)
        {
          if (folder->UnpackCRCDefined)
          {
            if (CrcCalc(*outBuffer, unpackSize) != folder->UnpackCRC)
              res = SZ_ERROR_CRC;
          }
        }
      }
    }
  }
  if (res == SZ_OK)
  {
    UInt32 i;
    CSzFileItem *fileItem = p->db.Files + fileIndex;
    *offset = 0;
    for (i = p->FolderStartFileIndex[folderIndex]; i < fileIndex; i++)
      *offset += (UInt32)p->db.Files[i].Size;
    *outSizeProcessed = (size_t)fileItem->Size;
    if (*offset + *outSizeProcessed > *outBufferSize)
      return SZ_ERROR_FAIL;
    {
      if (fileItem->FileCRCDefined)
      {
        if (CrcCalc(*outBuffer + *offset, *outSizeProcessed) != fileItem->FileCRC)
          res = SZ_ERROR_CRC;
      }
    }
  }
  return res;
}
