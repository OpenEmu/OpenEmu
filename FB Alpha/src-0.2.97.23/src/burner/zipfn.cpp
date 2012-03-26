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
