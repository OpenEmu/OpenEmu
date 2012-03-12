// CD/CD-ROM support
#include "burner.h"

bool bCDEmuOkay = false;
UINT32 nCDEmuSelect = 0;

CDEmuStatusValue CDEmuStatus;

static InterfaceInfo CDEmuInfo = { NULL, NULL, NULL };

#if defined BUILD_WIN32
	extern struct CDEmuDo isowavDo;
#elif defined BUILD_SDL
	// CD emulation module
#elif defined (_XBOX)
	extern struct CDEmuDo isowavDo;
#endif

static struct CDEmuDo* pCDEmuDo[] =
{
#if defined BUILD_WIN32
	&isowavDo,
#elif defined BUILD_SDL
	// CD emulation module
#elif defined (_XBOX)
	&isowavDo,
#endif
};

#define CDEMU_LEN (sizeof(pCDEmuDo) / sizeof(pCDEmuDo[0]))

TCHAR CDEmuImage[MAX_PATH] = _T("");


// ----------------------------------------------------------------------------

INT32 CDEmuExit()
{
	IntInfoFree(&CDEmuInfo);

	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}
	bCDEmuOkay = false;

	return pCDEmuDo[nCDEmuSelect]->CDEmuExit();
}

INT32 CDEmuInit()
{
	INT32 nRet;

	if (nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	CDEmuStatus = idle;

	if ((nRet = pCDEmuDo[nCDEmuSelect]->CDEmuInit()) == 0) {
		bCDEmuOkay = true;
	}

	return nRet;
}

INT32 CDEmuStop()
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuStop();
}

INT32 CDEmuPlay(UINT8 M, UINT8 S, UINT8 F)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuPlay(M, S, F);
}

INT32 CDEmuLoadSector(INT32 LBA, char* pBuffer)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 0;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuLoadSector(LBA, pBuffer);
}

UINT8* CDEmuReadTOC(INT32 track)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return NULL;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuReadTOC(track);
}

UINT8* CDEmuReadQChannel()
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return NULL;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuReadQChannel();
}

INT32 CDEmuGetSoundBuffer(INT16* buffer, INT32 samples)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuGetSoundBuffer(buffer, samples);
}

InterfaceInfo* CDEmuGetInfo()
{
	if (IntInfoInit(&CDEmuInfo)) {
		IntInfoFree(&CDEmuInfo);
		return NULL;
	}

	if (bCDEmuOkay) {

		CDEmuInfo.pszModuleName = pCDEmuDo[nCDEmuSelect]->szModuleName;

	 	if (pCDEmuDo[nCDEmuSelect]->GetPluginSettings) {
			pCDEmuDo[nCDEmuSelect]->GetPluginSettings(&CDEmuInfo);
		}
	} else {
		IntInfoAddStringInterface(&CDEmuInfo, _T("CD/CD-ROM emulation module not initialised"));
	}

	return &CDEmuInfo;

	return NULL;
}
