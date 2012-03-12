// Wave log module
#include "burner.h"


FILE *WaveLog=NULL; // wave log file


static void MakeOfn(TCHAR* pszFilter)
{
	_stprintf(pszFilter, FBALoadStringEx(hAppInst, IDS_DISK_FILE_SOUND, true), _T(APP_TITLE));
	memcpy(pszFilter + _tcslen(pszFilter), _T(" (*.wav)\0*.wav\0\0"), 16 * sizeof(TCHAR));

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = pszFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizeof(szChoice) / sizeof(TCHAR);
	ofn.lpstrInitialDir = _T(".\\wav");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("wav");

	return;
}


// Make a wave header so we can write out raw samples after it to make a wave file
static int WaveLogHeaderStart(FILE *Hand,int nSamRate)
{
	unsigned int ua=0x10;
	WORD  wFormatTag=1; // PCM
	WORD  nChannels=0;
	DWORD nSamplesPerSec=0,nAvgBytesPerSec=0;
	WORD  nBlockAlign=0,wBitsPerSample=0;

	nChannels=2; // stereo
	nSamplesPerSec=nSamRate;
	nAvgBytesPerSec=nSamRate*4; // samrate*2*short
	nBlockAlign=4; // 2*short
	wBitsPerSample=16; // 16-bit

	if (Hand==NULL)
		return 1;
	fwrite("RIFF    WAVEfmt ",1,0x10,Hand); // fill in 0x04 later
	fwrite(&ua,1,sizeof(ua),Hand);
	fwrite(&wFormatTag,1,sizeof(wFormatTag),Hand);
	fwrite(&nChannels,1,sizeof(nChannels),Hand);
	fwrite(&nSamplesPerSec,1,sizeof(nSamplesPerSec),Hand);
	fwrite(&nAvgBytesPerSec,1,sizeof(nAvgBytesPerSec),Hand);
	fwrite(&nBlockAlign,1,sizeof(nBlockAlign),Hand);
	fwrite(&wBitsPerSample,1,sizeof(wBitsPerSample),Hand);
	fwrite("data    ",1,0x08,Hand); // fill in 0x28 later
	return 0;
}

// Fill in the RIFF and data values
static int WaveLogHeaderFillIn(FILE *Hand)
{
	unsigned int nLen=0;
	if (Hand==NULL)
		return 1;
	fseek(Hand,0,SEEK_END); nLen=ftell(Hand);
	fseek(Hand,0x04,SEEK_SET); nLen-=8;  fwrite(&nLen,1,sizeof(nLen),Hand); // RIFF value
	fseek(Hand,0x28,SEEK_SET); nLen-=40; fwrite(&nLen,1,sizeof(nLen),Hand); // data value
	fseek(Hand,0,SEEK_END);
	return 0;
}

int WaveLogStart()
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

	WaveLogStop(); // make sure old log is closed

	MakeOfn(szFilter);
	bOldPause = bRunPause;
	bRunPause = 1;
	nRet = GetSaveFileName(&ofn);
	bRunPause = bOldPause;
	if (nRet == 0) {
		return 1;
	}

	{
		WaveLog=_tfopen(szChoice,_T("wb"));
		if (WaveLog==NULL)
		{
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_CREATE));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_SOUND));
			FBAPopupDisplay(PUF_TYPE_ERROR);
			return 1;
		}
		WaveLogHeaderStart(WaveLog,nAudSampleRate[nAudSelect]);

	}
	MenuEnableItems();
	return 0;
}

int WaveLogStop()
{
	if (WaveLog)
	{
		WaveLogHeaderFillIn(WaveLog);
		fclose(WaveLog);

		WaveLog = NULL;

		MenuEnableItems();
	}

	return 0;
}

