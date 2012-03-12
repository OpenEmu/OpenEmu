#ifndef _DSOUND_CORE_
#define _DSOUND_CORE_

#include <dsound.h>

// DirectSoundCreate
extern HRESULT (WINAPI *_DirectSoundCreate)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN);

INT32 DSCore_Init();

#endif
