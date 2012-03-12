// Profiling support
#include "burner.h"

bool bProfileOkay = false;
UINT32 nProfileSelect = 0;

static InterfaceInfo ProfileInfo = { NULL, NULL, NULL };

#if defined (BUILD_WIN32)
	extern struct ProfileDo cntDoPerfCount;
#elif defined (BUILD_SDL)
	extern struct ProfileDo cntDoPerfCount;
#elif defined (_XBOX)
	extern struct ProfileDo cntDoPerfCount;
#endif

static struct ProfileDo* pProfileDo[] =
{
#if defined (BUILD_WIN32)
	&cntDoPerfCount,
#elif defined (BUILD_SDL)
	&cntDoPerfCount,
#elif defined (_XBOX)
	&cntDoPerfCount,
#endif
};

#define PROFILE_LEN (sizeof(pProfileDo) / sizeof(pProfileDo[0]))

INT32 ProfileExit()
{
	IntInfoFree(&ProfileInfo);

	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 1;
	}
	bProfileOkay = false;

	return pProfileDo[nProfileSelect]->ProfileExit();
}

INT32 ProfileInit()
{
	INT32 nRet;

	if (nProfileSelect >= PROFILE_LEN) {
		return 1;
	}

	if ((nRet = pProfileDo[nProfileSelect]->ProfileInit()) == 0) {
		bProfileOkay = true;
	}

	return nRet;
}

INT32 ProfileProfileStart(INT32 nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 1;
	}

	return pProfileDo[nProfileSelect]->ProfileStart(nSubSystem);
}

INT32 ProfileProfileEnd(INT32 nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 1;
	}

	return pProfileDo[nProfileSelect]->ProfileEnd(nSubSystem);
}

double ProfileProfileReadLast(INT32 nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 0.0;
	}

	return pProfileDo[nProfileSelect]->ProfileReadLast(nSubSystem);
}

double ProfileProfileReadAverage(INT32 nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 0.0;
	}

	return pProfileDo[nProfileSelect]->ProfileReadAverage(nSubSystem);
}

InterfaceInfo* ProfileGetInfo()
{
	if (IntInfoInit(&ProfileInfo)) {
		IntInfoFree(&ProfileInfo);
		return NULL;
	}

	if (bProfileOkay) {

		ProfileInfo.pszModuleName = pProfileDo[nProfileSelect]->szModuleName;

	 	if (pProfileDo[nProfileSelect]->GetPluginSettings) {
			pProfileDo[nProfileSelect]->GetPluginSettings(&ProfileInfo);
		}
	} else {
		IntInfoAddStringInterface(&ProfileInfo, _T("Profiling module not initialised"));
	}

	return &ProfileInfo;
}
