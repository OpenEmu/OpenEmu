#include "burner.h"
#include "dsound_core.h"

// DirectSoundCreate
HRESULT (WINAPI *_DirectSoundCreate) (LPGUID, LPDIRECTSOUND*, LPUNKNOWN);
HRESULT WINAPI Empty_DirectSoundCreate (LPGUID, LPDIRECTSOUND*, LPUNKNOWN) { return 0; }

static HINSTANCE	hDSCore;
static BOOL			nDSCoreInit = FALSE;

/*
static void DSCore_Exit()
{
	FreeLibrary(hDSCore);
}
*/

// Macro for easy handling of functions
#define _LOADFN(_rettype, _name, _empty, _params, _hinst, _str )		\
																		\
	_name = (_rettype (WINAPI *)_params) GetProcAddress(_hinst, _str);	\
																		\
	if(!_name) {														\
		_name = _empty;													\
		return 0;														\
	}

static INT32 DSCore_GetFunctions()
{
	if(!nDSCoreInit) return 0;

	_LOADFN(HRESULT, _DirectSoundCreate, Empty_DirectSoundCreate, (LPGUID, LPDIRECTSOUND*, LPUNKNOWN), hDSCore, "DirectSoundCreate");

	return 1;
}

INT32 DSCore_Init()
{
	hDSCore = LoadLibrary(_T("dsound.dll"));

	if(!hDSCore) {
		MessageBox(NULL, _T("Loading of DSOUND.DLL failed."), _T("Error"), MB_OK | MB_ICONERROR);
		nDSCoreInit = FALSE;
		return 0;
	}
	
	nDSCoreInit = TRUE;
	if(!DSCore_GetFunctions()) {
		MessageBox(NULL, _T("There was a problem while loading functions from DSOUND.DLL"), _T("Error"), MB_OK | MB_ICONERROR);
		return 0;
	}

	return 1;
}
