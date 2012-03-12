// Zip module
#include "burner.h"
#include "unzip.h"

#ifdef INCLUDE_7Z_SUPPORT
#include "un7z.h"
#endif

#define ZIPFN_FILETYPE_NONE		-1
#define ZIPFN_FILETYPE_ZIP		1
#define ZIPFN_FILETYPE_7ZIP		2

static INT32 nFileType = ZIPFN_FILETYPE_NONE;

static unzFile Zip = NULL;
static INT32 nCurrFile = 0; // The current file we are pointing to

#ifdef INCLUDE_7Z_SUPPORT
static _7z_file* _7ZipFile = NULL;
#endif

#ifdef __LIBSNES_USE_ZLIB__
/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-2002 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

static const unsigned long crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1_CRC32(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2_CRC32(buf)  DO1_CRC32(buf); DO1_CRC32(buf);
#define DO4_CRC32(buf)  DO2_CRC32(buf); DO2_CRC32(buf);
#define DO8_CRC32(buf)  DO4_CRC32(buf); DO4_CRC32(buf);

static unsigned long  crc32(unsigned long crc, const unsigned char *buf, unsigned int len)
{
	if (buf == 0) return 0L;
	crc = crc ^ 0xffffffffL;
	while (len >= 8)
	{
		DO8_CRC32(buf);
		len -= 8;
	}
	if (len) do {
		DO1_CRC32(buf);
	} while (--len);
	return crc ^ 0xffffffffL;
}
#endif

INT32 ZipOpen(char* szZip)
{
	nFileType = ZIPFN_FILETYPE_NONE;
	
	if (szZip == NULL) return 1;
	
	char szFileName[MAX_PATH];
	
	sprintf(szFileName, "%s.zip", szZip);
	Zip = unzOpen(szFileName);
	if (Zip != NULL) {
		nFileType = ZIPFN_FILETYPE_ZIP;
		unzGoToFirstFile(Zip);
		nCurrFile = 0;
		
		return 0;
	}
	
#ifdef INCLUDE_7Z_SUPPORT
	sprintf(szFileName, "%s.7z", szZip);
	_7z_error _7zerr = 	_7z_file_open(szFileName, &_7ZipFile);
	if (_7zerr == _7ZERR_NONE) {
		nFileType = ZIPFN_FILETYPE_7ZIP;
		nCurrFile = 0;
		
		return 0;
	}
#endif

	return 1;
}

INT32 ZipClose()
{
	if (nFileType == ZIPFN_FILETYPE_ZIP) {
		if (Zip != NULL) {
			unzClose(Zip);
			Zip = NULL;
		}
	}

#ifdef INCLUDE_7Z_SUPPORT
	if (nFileType == ZIPFN_FILETYPE_7ZIP) {
		if (_7ZipFile != NULL) {
			_7z_file_close(_7ZipFile);
			_7ZipFile = NULL;
		}
	}
#endif
	
	nFileType = ZIPFN_FILETYPE_NONE;
	
	return 0;
}

// Get the contents of a zip file into an array of ZipEntrys
INT32 ZipGetList(struct ZipEntry** pList, INT32* pnListCount)
{
	if (nFileType == ZIPFN_FILETYPE_ZIP && Zip == NULL) return 1;
	if (pList == NULL) return 1;
	
#ifdef INCLUDE_7Z_SUPPORT
	if (nFileType == ZIPFN_FILETYPE_7ZIP && _7ZipFile == NULL) return 1;	
#endif
	
	if (nFileType == ZIPFN_FILETYPE_ZIP) {
		unz_global_info ZipGlobalInfo;
		memset(&ZipGlobalInfo, 0, sizeof(ZipGlobalInfo));
		
		unzGetGlobalInfo(Zip, &ZipGlobalInfo);
		INT32 nListLen = ZipGlobalInfo.number_entry;

		// Make an array of File Entries
		struct ZipEntry* List = (struct ZipEntry *)malloc(nListLen * sizeof(struct ZipEntry));
		if (List == NULL) { unzClose(Zip); return 1; }
		memset(List, 0, nListLen * sizeof(struct ZipEntry));

		INT32 nRet = unzGoToFirstFile(Zip);
		if (nRet != UNZ_OK) { unzClose(Zip); return 1; }

		// Step through all of the files, until we get to the end
		INT32 nNextRet = 0;

		for (nCurrFile = 0, nNextRet = UNZ_OK;
			nCurrFile < nListLen && nNextRet == UNZ_OK;
			nCurrFile++, nNextRet = unzGoToNextFile(Zip))
		{
			unz_file_info FileInfo;
			memset(&FileInfo, 0, sizeof(FileInfo));

			nRet = unzGetCurrentFileInfo(Zip, &FileInfo, NULL, 0, NULL, 0, NULL, 0);
			if (nRet != UNZ_OK) continue;

			// Allocate space for the filename
			char* szName = (char *)malloc(FileInfo.size_filename + 1);
			if (szName == NULL) continue;

			nRet = unzGetCurrentFileInfo(Zip, &FileInfo, szName, FileInfo.size_filename + 1, NULL, 0, NULL, 0);
			if (nRet != UNZ_OK) continue;

			List[nCurrFile].szName = szName;
			List[nCurrFile].nLen = FileInfo.uncompressed_size;
			List[nCurrFile].nCrc = FileInfo.crc;
		}

		// return the file list
		*pList = List;
		if (pnListCount != NULL) *pnListCount = nListLen;

		unzGoToFirstFile(Zip);
		nCurrFile = 0;
	}
	
#ifdef INCLUDE_7Z_SUPPORT
	if (nFileType == ZIPFN_FILETYPE_7ZIP) {
		UInt16 *temp = NULL;
		size_t tempSize = 0;
		
		INT32 nListLen = _7ZipFile->db.db.NumFiles;

		// Make an array of File Entries
		struct ZipEntry* List = (struct ZipEntry *)malloc(nListLen * sizeof(struct ZipEntry));
		if (List == NULL) return 1;
		memset(List, 0, nListLen * sizeof(struct ZipEntry));
		
		for (UINT32 i = 0; i < _7ZipFile->db.db.NumFiles; i++) {
			const CSzFileItem *f = _7ZipFile->db.db.Files + i;
			
			size_t len = SzArEx_GetFileNameUtf16(&_7ZipFile->db, i, NULL);

			// if it's a directory entry we don't care about it..
			if (f->IsDir) continue;

			if (len > tempSize) {
				SZipFree(NULL, temp);
				tempSize = len;
				temp = (UInt16 *)SZipAlloc(NULL, tempSize * sizeof(temp[0]));
				if (temp == 0) {
					return 1; // memory error
				}
			}
			
			UINT64 size = f->Size;
			UINT32 crc = f->Crc;
			
			SzArEx_GetFileNameUtf16(&_7ZipFile->db, i, temp);
			
			// convert filename to char
			char *szFileName = NULL;
			szFileName = (char*)malloc(len * 2 * sizeof(char*));
			if (szFileName == NULL) continue;
			
			for (UINT32 j = 0; j < len; j++) {
				szFileName[j + 0] = temp[j] & 0xff;
				szFileName[j + 1] = temp[j] >> 8;
			}
			
			List[nCurrFile].szName = szFileName;
			List[nCurrFile].nLen = size;
			List[nCurrFile].nCrc = crc;
			
			nCurrFile++;
		}
		
		// return the file list
		*pList = List;
		if (pnListCount != NULL) *pnListCount = nListLen;
		
		nCurrFile = 0;
		
		SZipFree(NULL, temp);
	}
#endif
		
	return 0;
}

INT32 ZipLoadFile(UINT8* Dest, INT32 nLen, INT32* pnWrote, INT32 nEntry)
{
	if (nFileType == ZIPFN_FILETYPE_ZIP && Zip == NULL) return 1;

#ifdef INCLUDE_7Z_SUPPORT
	if (nFileType == ZIPFN_FILETYPE_7ZIP && _7ZipFile == NULL) return 1;	
#endif

	INT32 nRet = 0;
	
	if (nFileType == ZIPFN_FILETYPE_ZIP) {
		if (nEntry < nCurrFile)
		{
			// We'll have to go through the zip file again to get to our entry
			nRet = unzGoToFirstFile(Zip);
			if (nRet != UNZ_OK) return 1;
			nCurrFile = 0;
		}

		// Now step through to the file we need
		while (nCurrFile < nEntry)
		{
			nRet = unzGoToNextFile(Zip);
			if (nRet != UNZ_OK) return 1;
			nCurrFile++;
		}

		nRet = unzOpenCurrentFile(Zip);
		if (nRet != UNZ_OK) return 1;

		nRet = unzReadCurrentFile(Zip, Dest, nLen);
		// Return how many bytes were copied
		if (nRet >= 0 && pnWrote != NULL) *pnWrote = nRet;

		nRet = unzCloseCurrentFile(Zip);
		if (nRet == UNZ_CRCERROR) return 2;
		if (nRet != UNZ_OK) return 1;
	}
	
#ifdef INCLUDE_7Z_SUPPORT
	if (nFileType == ZIPFN_FILETYPE_7ZIP) {
		_7ZipFile->curr_file_idx = nEntry;
		UINT32 nWrote = 0;
		
		const CSzFileItem *f = _7ZipFile->db.db.Files + nEntry;
		
		_7z_error _7zerr = _7z_file_decompress(_7ZipFile, Dest, nLen, &nWrote);
		if (_7zerr != _7ZERR_NONE) return 1;
		
		// Return how many bytes were copied
		if (_7zerr == _7ZERR_NONE && pnWrote != NULL) *pnWrote = (INT32)nWrote;
		
		// use zlib crc32 module to calc crc of decompressed data, and check against 7z header
		UINT32 nCalcCrc = crc32(0, Dest, nWrote);
		if (nCalcCrc != f->Crc) return 2;
	}
#endif

	return 0;
}

// Load one file directly, added by regret
INT32 __cdecl ZipLoadOneFile(char* arcName, const char* fileName, void** Dest, INT32* pnWrote)
{
	if (ZipOpen(arcName)) {
		return 1;
	}

	if (nFileType == ZIPFN_FILETYPE_ZIP) {
		unz_global_info ZipGlobalInfo;
		memset(&ZipGlobalInfo, 0, sizeof(ZipGlobalInfo));

		unzGetGlobalInfo(Zip, &ZipGlobalInfo);
		INT32 nListLen = ZipGlobalInfo.number_entry;
		if (nListLen <= 0) {
			ZipClose();
			return 1;
		}

		INT32 nRet = unzGoToFirstFile(Zip);
		if (nRet != UNZ_OK) { unzClose(Zip); return 1; }

		unz_file_info FileInfo;
		memset(&FileInfo, 0, sizeof(FileInfo));

		if (fileName != NULL) {
			// Step through all of the files, until we get to the end
			INT32 nNextRet = 0;
			char szName[MAX_PATH] = "";

			for (nCurrFile = 0, nNextRet = UNZ_OK;
				nCurrFile < nListLen && nNextRet == UNZ_OK;
				nCurrFile++, nNextRet = unzGoToNextFile(Zip))
			{
				nRet = unzGetCurrentFileInfo(Zip, &FileInfo, szName, MAX_PATH, NULL, 0, NULL, 0);
				if (nRet != UNZ_OK) continue;

				if (!strcmp(szName, fileName)) {
					break;
				}
			}

			if (nCurrFile == nListLen) {
				ZipClose();
				return 1; // didn't find
			}
		}
		else {
			nRet = unzGetCurrentFileInfo(Zip, &FileInfo, NULL, 0, NULL, 0, NULL, 0);
			if (nRet != UNZ_OK) {
				ZipClose();
				return 1;
			}
		}

		// Extract file
		nRet = unzOpenCurrentFile(Zip);
		if (nRet != UNZ_OK) {
			unzCloseCurrentFile(Zip);
			ZipClose();
			return 1;
		}

		if (*Dest == NULL) {
			*Dest = (UINT8*)malloc(FileInfo.uncompressed_size);
			if (!*Dest) {
				unzCloseCurrentFile(Zip);
				ZipClose();
				return 1;
			}
		}

		nRet = unzReadCurrentFile(Zip, *Dest, FileInfo.uncompressed_size);
		// Return how many bytes were copied
		if (nRet >= 0 && pnWrote != NULL) *pnWrote = nRet;

		nRet = unzCloseCurrentFile(Zip);
		ZipClose();

		if (nRet == UNZ_CRCERROR) {
			if (*Dest) {
				free(*Dest);
			}
			return 2;
		}
		if (nRet != UNZ_OK) {
			if (*Dest) {
				free(*Dest);
			}
			return 1;
		}
	}
	
#ifdef INCLUDE_7Z_SUPPORT
	if (nFileType == ZIPFN_FILETYPE_7ZIP) {
		UINT32 nWrote = 0;
		
		nCurrFile = _7z_search_crc_match(_7ZipFile, 0, fileName, strlen(fileName), 0, 1);
		if (nCurrFile == -1) {
			ZipClose();
			return 1;
		}
		
		const CSzFileItem *f = _7ZipFile->db.db.Files + nCurrFile;
		
		_7ZipFile->curr_file_idx = nCurrFile;
		
		if (*Dest == NULL) {
			*Dest = (UINT8*)malloc(f->Size);
			if (!*Dest) {
				ZipClose();
				return 1;
			}
		}
		
		_7z_error _7zerr = _7z_file_decompress(_7ZipFile, *Dest, f->Size, &nWrote);
		if (_7zerr != _7ZERR_NONE) {
			ZipClose();
			if (*Dest) free(*Dest);
			return 1;
		}
		
		// Return how many bytes were copied
		if (_7zerr == _7ZERR_NONE && pnWrote != NULL) *pnWrote = (INT32)nWrote;
		
		// use zlib crc32 module to calc crc of decompressed data, and check against 7z header
		UINT32 nCalcCrc = crc32(0, (const Bytef*)*Dest, nWrote);
		if (nCalcCrc != f->Crc) {
			ZipClose();
			if (*Dest) free(*Dest);
			return 2;
		}
		
		ZipClose();
		
		nCurrFile = 0;		
	}	
#endif

	return 0;
}
