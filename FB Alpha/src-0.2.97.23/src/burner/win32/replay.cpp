// Functions for recording & replaying input
#include "burner.h"
#include "dynhuff.h"
#include <commdlg.h>

int nReplayStatus = 0;

static unsigned int nStartFrame;
static unsigned int nEndFrame;

static FILE* fp = NULL;
static int nSizeOffset;

static short nPrevInputs[0x0100];

int RecordInput()
{
	struct BurnInputInfo bii;
	memset(&bii, 0, sizeof(bii));

	for (unsigned int i = 0; i < nGameInpCount; i++) {
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal) {
			if (bii.nType & BIT_GROUP_ANALOG) {
				if (*bii.pShortVal != nPrevInputs[i]) {
					EncodeBuffer(i);
					EncodeBuffer(*bii.pShortVal >> 8);
					EncodeBuffer(*bii.pShortVal & 0xFF);
					nPrevInputs[i] = *bii.pShortVal;
				}
			} else {
				if (*bii.pVal != nPrevInputs[i]) {
					EncodeBuffer(i);
					EncodeBuffer(*bii.pVal);
					nPrevInputs[i] = *bii.pVal;
				}
			}
		}
	}

	EncodeBuffer(0xFF);
	return 0;
}

int ReplayInput()
{
	unsigned char n;
	struct BurnInputInfo bii;
	memset(&bii, 0, sizeof(bii));

	// Just to be safe, restore the inputs to the known correct settings
	for (unsigned int i = 0; i < nGameInpCount; i++) {
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal) {
			if (bii.nType & BIT_GROUP_ANALOG) {
				*bii.pShortVal = nPrevInputs[i];
			} else {
				*bii.pVal = nPrevInputs[i];
			}
		}
	}

	// Now read all inputs that need to change from the .fr file
	while ((n = DecodeBuffer()) != 0xFF) {
		BurnDrvGetInputInfo(&bii, n);
		if (bii.pVal) {
			if (bii.nType & BIT_GROUP_ANALOG) {
				*bii.pShortVal = nPrevInputs[n] = (DecodeBuffer() << 8) | DecodeBuffer();
			} else {
				*bii.pVal = nPrevInputs[n] = DecodeBuffer();
			}
		} else {
			DecodeBuffer();
		}
	}

	if (end_of_buffer) {
		StopReplay();
		return 1;
	} else {
		return 0;
	}
}

static void MakeOfn(TCHAR* pszFilter)
{
	_stprintf(pszFilter, FBALoadStringEx(hAppInst, IDS_DISK_FILE_REPLAY, true), _T(APP_TITLE));
	memcpy(pszFilter + _tcslen(pszFilter), _T(" (*.fr)\0*.fr\0\0"), 14 * sizeof(TCHAR));

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = pszFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizeof(szChoice) / sizeof(TCHAR);
	ofn.lpstrInitialDir = _T(".\\recordings");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("fr");

	return;
}

int StartRecord()
{
	TCHAR szFilter[1024];

	int nRet;
	int bOldPause;

	fp = NULL;

	_stprintf(szChoice, _T("%s"), BurnDrvGetText(DRV_NAME));
	MakeOfn(szFilter);
	ofn.lpstrTitle = FBALoadStringEx(hAppInst, IDS_REPLAY_RECORD, true);
	ofn.Flags |= OFN_OVERWRITEPROMPT;

	bOldPause = bRunPause;
	bRunPause = 1;
	nRet = GetSaveFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}

	{
		const char szFileHeader[] = "FB1 ";				// File identifier
		fp = _tfopen(szChoice, _T("wb"));

		nRet = 0;
		if (fp == NULL) {
			nRet = 1;
		} else {
			fwrite(&szFileHeader, 1, 4, fp);
			nRet = BurnStateSaveEmbed(fp, -1, 1);
			if (nRet >= 0) {
				const char szChunkHeader[] = "FR1 ";	// Chunk identifier
				int nZero = 0;

				fwrite(&szChunkHeader, 1, 4, fp);		// Write chunk identifier

				nSizeOffset = ftell(fp);

				fwrite(&nZero, 1, 4, fp);				// reserve space for chunk size

				fwrite(&nZero, 1, 4, fp);				// reserve space for number of frames

				fwrite(&nZero, 1, 4, fp);				// reserved
				fwrite(&nZero, 1, 4, fp);				//
				fwrite(&nZero, 1, 4, fp);				//

				nRet = EmbedCompressedFile(fp, -1);
			}
		}
	}

	if (nRet) {
		if (fp) {
			fclose(fp);
			fp = NULL;
		}

		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_CREATE));
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_REPLAY));
		FBAPopupDisplay(PUF_TYPE_ERROR);
		return 1;
	} else {
		struct BurnInputInfo bii;
		memset(&bii, 0, sizeof(bii));

		nReplayStatus = 1;								// Set record status
		MenuEnableItems();

		nStartFrame = GetCurrentFrame();

		// Create a baseline so we can just record the deltas afterwards
		for (unsigned int i = 0; i < nGameInpCount; i++) {
			BurnDrvGetInputInfo(&bii, i);
			if (bii.pVal) {
				if (bii.nType & BIT_GROUP_ANALOG) {
					EncodeBuffer(*bii.pShortVal >> 8);
					EncodeBuffer(*bii.pShortVal & 0xFF);
					nPrevInputs[i] = *bii.pShortVal;
				} else {
					EncodeBuffer(*bii.pVal);
					nPrevInputs[i] = *bii.pVal;
				}
			} else {
				EncodeBuffer(0);
			}
		}

#ifdef FBA_DEBUG
		dprintf(_T("*** Recording of file %s started.\n"), szChoice);
#endif

		return 0;
	}
}

int StartReplay(const TCHAR* szFileName)					// const char* szFileName = NULL
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

	fp = NULL;

	if (szFileName) {
		_tcscpy(szChoice, szFileName);
	} else {
		if (bDrvOkay) {
			_stprintf(szChoice, _T("%s*.fr"), BurnDrvGetText(DRV_NAME));
		} else {
			_stprintf(szChoice, _T("recording"));
		}
		MakeOfn(szFilter);
		ofn.lpstrTitle = FBALoadStringEx(hAppInst, IDS_REPLAY_REPLAY, true);

		bOldPause = bRunPause;
		bRunPause = 1;
		nRet = GetOpenFileName(&ofn);
		bRunPause = bOldPause;

		if (nRet == 0) {
			return 1;
		}
	}

	{
		const char szFileHeader[] = "FB1 ";					// File identifier
		char ReadHeader[] = "    ";
		fp = _tfopen(szChoice, _T("rb"));
		memset(ReadHeader, 0, 4);
		fread(ReadHeader, 1, 4, fp);						// Read identifier
		if (memcmp(ReadHeader, szFileHeader, 4)) {			// Not the right file type
			fclose(fp);
			fp = NULL;
			nRet = 2;
		} else {
			// First load the savestate associated with the recording
			nRet = BurnStateLoadEmbed(fp, -1, 1, &DrvInitCallback);
			if (nRet == 0) {
				const char szChunkHeader[] = "FR1 ";		// Chunk identifier
				fread(ReadHeader, 1, 4, fp);				// Read identifier
				if (memcmp(ReadHeader, szChunkHeader, 4)) {	// Not the right file type
					fclose(fp);
					fp = NULL;
					nRet = 2;
				} else {
					// Open the recording itself
					fseek(fp, 0x04, SEEK_CUR);				// Skip chunk size
					fread(&nEndFrame, 1, 4, fp);			// Read framecount
					nStartFrame = GetCurrentFrame();
					nEndFrame += nStartFrame;
					fseek(fp, 0x0C, SEEK_CUR);				// Skip rest of header
					nRet = EmbedCompressedFile(fp, -1);

				}
			}
		}

		// Describe any possible errors:
		if (nRet == 3) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_THIS_REPLAY));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_UNAVAIL));
		} else {
			if (nRet == 4) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_THIS_REPLAY));
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_TOOOLD), _T(APP_TITLE));
			} else {
				if (nRet == 5) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_THIS_REPLAY));
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_TOONEW), _T(APP_TITLE));
				} else {
					if (nRet) {
						FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_LOAD));
						FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_REPLAY));
					}
				}
			}
		}

		if (nRet) {
			if (fp) {
				fclose(fp);
				fp = NULL;
			}

			FBAPopupDisplay(PUF_TYPE_ERROR);

			return 1;
		}
	}

	nReplayStatus = 2;							// Set replay status
	MenuEnableItems();

	{
		struct BurnInputInfo bii;
		memset(&bii, 0, sizeof(bii));

		LoadCompressedFile();

		// Get the baseline
		for (unsigned int i = 0; i < nGameInpCount; i++) {
			BurnDrvGetInputInfo(&bii, i);
			if (bii.pVal) {
				if (bii.nType & BIT_GROUP_ANALOG) {
					*bii.pShortVal = nPrevInputs[i] = (DecodeBuffer() << 8) | DecodeBuffer();

				} else {
					*bii.pVal = nPrevInputs[i] = DecodeBuffer();
				}
			} else {
				DecodeBuffer();
			}
		}
	}

#ifdef FBA_DEBUG
	dprintf(_T("*** Replay of file %s started.\n"), szChoice);
#endif

	return 0;
}

static void CloseRecord()
{
	int nFrames = GetCurrentFrame() - nStartFrame;

	WriteCompressedFile();

	fseek(fp, 0, SEEK_END);
	int nChunkSize = ftell(fp) - 4 - nSizeOffset;		// Fill in chunk size and no of recorded frames
	fseek(fp, nSizeOffset, SEEK_SET);
	fwrite(&nChunkSize, 1, 4, fp);
	fwrite(&nFrames, 1, 4, fp);

	fclose(fp);
	fp = NULL;
}

static void CloseReplay()
{
	CloseCompressedFile();

	fclose(fp);
	fp = NULL;
}

void StopReplay()
{
	if (nReplayStatus) {
		if (nReplayStatus == 1) {

#ifdef FBA_DEBUG
			dprintf(_T(" ** Recording stopped, recorded %d frames.\n"), GetCurrentFrame() - nStartFrame);
#endif
			CloseRecord();
#ifdef FBA_DEBUG
			PrintResult();
#endif
		} else {
#ifdef FBA_DEBUG
			dprintf(_T(" ** Replay stopped, replayed %d frames.\n"), GetCurrentFrame() - nStartFrame);
#endif

			CloseReplay();
		}
		nReplayStatus = 0;
		MenuEnableItems();
	}
}

