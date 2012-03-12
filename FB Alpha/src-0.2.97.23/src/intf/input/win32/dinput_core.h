#ifndef _DINPUT_CORE_
#define _DINPUT_CORE_

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

extern const DIDATAFORMAT c_dfDIJoystick2;
extern const DIDATAFORMAT c_dfDIMouse2;
extern const DIDATAFORMAT c_dfDIKeyboard;

// DirectInput8Create
extern HRESULT (WINAPI* _DirectInput8Create) (HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);

INT32 DICore_Init();

#endif
