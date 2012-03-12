#ifndef _DDRAW_CORE_
#define _DDRAW_CORE_

#include <ddraw.h>

// DirectDrawCreateEx
extern HRESULT (WINAPI* _DirectDrawCreateEx) (GUID FAR*, LPVOID*, REFIID, IUnknown FAR*);

// DirectDrawCreateClipper
extern HRESULT (WINAPI* _DirectDrawCreateClipper) (DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR*);

// DirectDrawEnumerateEx
#ifdef UNICODE
extern HRESULT (WINAPI* _DirectDrawEnumerateEx) (LPDDENUMCALLBACKEXW, LPVOID, DWORD);
#else
extern HRESULT (WINAPI* _DirectDrawEnumerateEx) (LPDDENUMCALLBACKEXA, LPVOID, DWORD);
#endif

INT32 DDCore_Init();

#endif
