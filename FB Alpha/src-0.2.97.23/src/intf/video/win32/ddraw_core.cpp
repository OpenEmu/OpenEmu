#include "burner.h"
#include "ddraw_core.h"

// DirectDrawCreateEx
HRESULT (WINAPI* _DirectDrawCreateEx) (GUID FAR*, LPVOID*, REFIID, IUnknown FAR*);
HRESULT WINAPI Empty_DirectDrawCreateEx (GUID FAR*, LPVOID*, REFIID, IUnknown FAR*) { return 0; }

// DirectDrawCreateClipper
HRESULT (WINAPI* _DirectDrawCreateClipper) (DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR*); 
HRESULT WINAPI Empty_DirectDrawCreateClipper (DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR*) { return 0; }

// DirectDrawEnumerateEx
#ifdef UNICODE
HRESULT (WINAPI* _DirectDrawEnumerateEx) (LPDDENUMCALLBACKEXW, LPVOID, DWORD);
HRESULT WINAPI Empty_DirectDrawEnumerateEx (LPDDENUMCALLBACKEXW, LPVOID, DWORD) { return 0; }
#else
HRESULT (WINAPI* _DirectDrawEnumerateEx) (LPDDENUMCALLBACKEXA, LPVOID, DWORD);
HRESULT WINAPI Empty_DirectDrawEnumerateEx (LPDDENUMCALLBACKEXA, LPVOID, DWORD) { return 0; }
#endif

static HINSTANCE	hDDCore;
static BOOL			nDDCoreInit = FALSE;

/*
static void DDCore_Exit()
{
	FreeLibrary(hDDCore);
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

static INT32 DDCore_GetFunctions()
{
	if(!nDDCoreInit) return 0;

	_LOADFN(HRESULT, _DirectDrawCreateEx, Empty_DirectDrawCreateEx, (GUID FAR*, LPVOID*, REFIID, IUnknown FAR*), hDDCore, "DirectDrawCreateEx");
	_LOADFN(HRESULT, _DirectDrawCreateClipper, Empty_DirectDrawCreateClipper, (DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR*), hDDCore, "DirectDrawCreateClipper");
#ifdef UNICODE
	_LOADFN(HRESULT, _DirectDrawEnumerateEx, Empty_DirectDrawEnumerateEx, (LPDDENUMCALLBACKEXW, LPVOID, DWORD), hDDCore, "DirectDrawEnumerateExW");
#else
	_LOADFN(HRESULT, _DirectDrawEnumerateEx, Empty_DirectDrawEnumerateEx, (LPDDENUMCALLBACKEXA, LPVOID, DWORD), hDDCore, "DirectDrawEnumerateExA");
#endif
	return 1;
}

INT32 DDCore_Init()
{
	hDDCore = LoadLibrary(_T("ddraw.dll"));

	if(!hDDCore) {
		MessageBox(NULL, _T("Loading of DDRAW.DLL failed."), _T("Error"), MB_OK | MB_ICONERROR);
		nDDCoreInit = FALSE;
		return 0;
	}
	
	nDDCoreInit = TRUE;
	if(!DDCore_GetFunctions()) {
		MessageBox(NULL, _T("There was a problem while loading functions from DDRAW.DLL"), _T("Error"), MB_OK | MB_ICONERROR);
		return 0;
	}

	return 1;
}
