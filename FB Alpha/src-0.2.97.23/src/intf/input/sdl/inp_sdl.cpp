// Module for input using SDL
#include <SDL.h>

#include "burner.h"
#include "inp_sdl_keys.h"

#define MAX_JOYSTICKS (8)

static int FBKtoSDL[512];

static int nInitedSubsytems = 0;
static SDL_Joystick* JoyList[MAX_JOYSTICKS];
static int* JoyPrevAxes = NULL;
static int nJoystickCount = 0;						// Number of joysticks connected to this machine

// Sets up one Joystick (for example the range of the joystick's axes)
static int SDLinpJoystickInit(int i)
{
	JoyList[i] = SDL_JoystickOpen(i);
	return 0;
}

// Set up the keyboard
static int SDLinpKeyboardInit()
{
	for (int i = 0; i < 512; i++) {
		FBKtoSDL[SDLtoFBK[i]] = i;
	}

	return 0;
}

// Get an interface to the mouse
static int SDLinpMouseInit()
{
	return 0;
}

int SDLinpSetCooperativeLevel(bool bExclusive, bool /*bForeGround*/)
{
	SDL_WM_GrabInput((bDrvOkay && (bExclusive || nVidFullscreen)) ? SDL_GRAB_ON : SDL_GRAB_OFF);
	SDL_ShowCursor((bDrvOkay && (bExclusive || nVidFullscreen)) ? SDL_DISABLE : SDL_ENABLE);

	return 0;
}

int SDLinpExit()
{
	// Close all joysticks
	for (int i = 0; i < MAX_JOYSTICKS; i++) {
		if (JoyList[i]) {
			SDL_JoystickClose(JoyList[i]);
			JoyList[i] = NULL;
		}
	}

	nJoystickCount = 0;

	free(JoyPrevAxes);
	JoyPrevAxes = NULL;

	if (!(nInitedSubsytems & SDL_INIT_JOYSTICK)) {
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
	if (!(nInitedSubsytems & SDL_INIT_VIDEO)) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
	nInitedSubsytems = 0;

//	SDL_Quit();

	return 0;
}

int SDLinpInit()
{
	int nSize;

	SDLinpExit();

	memset(&JoyList, 0, sizeof(JoyList));

	nSize = MAX_JOYSTICKS * 8 * sizeof(int);
	if ((JoyPrevAxes = (int*)malloc(nSize)) == NULL) {
		SDLinpExit();
		return 1;
	}
	memset(JoyPrevAxes, 0, nSize);

//	SDL_Init(0);

	nInitedSubsytems = SDL_WasInit(SDL_INIT_EVERYTHING);

	if (!(nInitedSubsytems & SDL_INIT_VIDEO)) {
		SDL_InitSubSystem(SDL_INIT_VIDEO);
	}
	if (!(nInitedSubsytems & SDL_INIT_JOYSTICK)) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	}

//	SDL_SetVideoMode(320, 224, 0, SDL_RESIZABLE | SDL_HWSURFACE);

	// Set up the joysticks
	nJoystickCount = SDL_NumJoysticks();
	for (int i = 0; i < nJoystickCount; i++) {
		SDLinpJoystickInit(i);
	}
	SDL_JoystickEventState(SDL_IGNORE);

	// Set up the keyboard
	SDLinpKeyboardInit();

	// Set up the mouse
	SDLinpMouseInit();

	return 0;
}

static unsigned char bKeyboardRead = 0;
static unsigned char* SDLinpKeyboardState;

static unsigned char bJoystickRead = 0;

static unsigned char bMouseRead = 0;
static struct { unsigned char buttons; int xdelta; int ydelta; } SDLinpMouseState;

#define SDL_KEY_DOWN(key) (FBKtoSDL[key] > 0 ? SDLinpKeyboardState[FBKtoSDL[key]] : 0)

// Call before checking for Input in a frame
int SDLinpStart()
{
	// Update SDL event queue
	SDL_PumpEvents();

	// Keyboard not read this frame
	bKeyboardRead = 0;

	// No joysticks have been read for this frame
	bJoystickRead = 0;

	// Mouse not read this frame
	bMouseRead = 0;

	return 0;
}

// Read one of the joysticks
static int ReadJoystick()
{
	if (bJoystickRead) {
		return 0;
	}

	SDL_JoystickUpdate();

	// All joysticks have been Read this frame
	bJoystickRead = 1;

	return 0;
}

// Read one joystick axis
int SDLinpJoyAxis(int i, int nAxis)
{
	if (i < 0 || i >= nJoystickCount) {				// This joystick number isn't connected
		return 0;
	}

	if (ReadJoystick() != 0) {						// There was an error polling the joystick
		return 0;
	}

	if (nAxis >= SDL_JoystickNumAxes(JoyList[i])) {
		return 0;
	}

	return SDL_JoystickGetAxis(JoyList[i], nAxis) << 1;
}

// Read the keyboard
static int ReadKeyboard()
{
	int numkeys;

	if (bKeyboardRead) {							// already read this frame - ready to go
		return 0;
	}

	SDLinpKeyboardState = SDL_GetKeyState(&numkeys);
	if (SDLinpKeyboardState == NULL) {
		return 1;
	}

	// The keyboard has been successfully Read this frame
	bKeyboardRead = 1;

	return 0;
}

static int ReadMouse()
{
	if (bMouseRead) {
		return 0;
	}

	SDLinpMouseState.buttons = SDL_GetRelativeMouseState(&(SDLinpMouseState.xdelta), &(SDLinpMouseState.ydelta));

	bMouseRead = 1;

	return 0;
}

// Read one mouse axis
int SDLinpMouseAxis(int i, int nAxis)
{
	if (i < 0 || i >= 1) {									// Only the system mouse is supported by SDL
		return 0;
	}

	switch (nAxis) {
		case 0:
			return SDLinpMouseState.xdelta;
		case 1:
			return SDLinpMouseState.ydelta;
	}

	return 0;
}

// Check a subcode (the 40xx bit in 4001, 4102 etc) for a joystick input code
static int JoystickState(int i, int nSubCode)
{
	if (i < 0 || i >= nJoystickCount) {							// This joystick isn't connected
		return 0;
	}

	if (ReadJoystick() != 0) {									// Error polling the joystick
		return 0;
	}

	if (nSubCode < 0x10) {										// Joystick directions
		const int DEADZONE = 0x4000;

		if (SDL_JoystickNumAxes(JoyList[i]) <= nSubCode) {
			return 0;
		}

		switch (nSubCode) {
			case 0x00: return SDL_JoystickGetAxis(JoyList[i], 0) < -DEADZONE;		// Left
			case 0x01: return SDL_JoystickGetAxis(JoyList[i], 0) > DEADZONE;		// Right
			case 0x02: return SDL_JoystickGetAxis(JoyList[i], 1) < -DEADZONE;		// Up
			case 0x03: return SDL_JoystickGetAxis(JoyList[i], 1) > DEADZONE;		// Down
			case 0x04: return SDL_JoystickGetAxis(JoyList[i], 2) < -DEADZONE;
			case 0x05: return SDL_JoystickGetAxis(JoyList[i], 2) > DEADZONE;
			case 0x06: return SDL_JoystickGetAxis(JoyList[i], 3) < -DEADZONE;
			case 0x07: return SDL_JoystickGetAxis(JoyList[i], 3) > DEADZONE;
			case 0x08: return SDL_JoystickGetAxis(JoyList[i], 4) < -DEADZONE;
			case 0x09: return SDL_JoystickGetAxis(JoyList[i], 4) > DEADZONE;
			case 0x0A: return SDL_JoystickGetAxis(JoyList[i], 5) < -DEADZONE;
			case 0x0B: return SDL_JoystickGetAxis(JoyList[i], 5) > DEADZONE;
			case 0x0C: return SDL_JoystickGetAxis(JoyList[i], 6) < -DEADZONE;
			case 0x0D: return SDL_JoystickGetAxis(JoyList[i], 6) > DEADZONE;
			case 0x0E: return SDL_JoystickGetAxis(JoyList[i], 7) < -DEADZONE;
			case 0x0F: return SDL_JoystickGetAxis(JoyList[i], 7) > DEADZONE;
		}
	}
	if (nSubCode < 0x20) {										// POV hat controls
		if (SDL_JoystickNumHats(JoyList[i]) <= ((nSubCode & 0x0F) >> 2)) {
			return 0;
		}

		switch (nSubCode & 3) {
			case 0:												// Left
				return SDL_JoystickGetHat(JoyList[i], (nSubCode & 0x0F) >> 2) & SDL_HAT_LEFT;
			case 1:												// Right
				return SDL_JoystickGetHat(JoyList[i], (nSubCode & 0x0F) >> 2) & SDL_HAT_RIGHT;
			case 2:												// Up
				return SDL_JoystickGetHat(JoyList[i], (nSubCode & 0x0F) >> 2) & SDL_HAT_UP;
			case 3:												// Down
				return SDL_JoystickGetHat(JoyList[i], (nSubCode & 0x0F) >> 2) & SDL_HAT_DOWN;
		}

		return 0;
	}
	if (nSubCode < 0x80) {										// Undefined
		return 0;
	}
	if (nSubCode < 0x80 + SDL_JoystickNumButtons(JoyList[i])) {	// Joystick buttons
		return SDL_JoystickGetButton(JoyList[i], nSubCode & 0x7F);
	}

	return 0;
}

// Check a subcode (the 80xx bit in 8001, 8102 etc) for a mouse input code
static int CheckMouseState(unsigned int nSubCode)
{
	switch (nSubCode & 0x7F) {
		case 0:
			return (SDLinpMouseState.buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
		case 1:
			return (SDLinpMouseState.buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		case 2:
			return (SDLinpMouseState.buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
	}

	return 0;
}

// Get the state (pressed = 1, not pressed = 0) of a particular input code
int SDLinpState(int nCode)
{
	if (nCode < 0) {
		return 0;
	}

	if (nCode < 0x100) {
		if (ReadKeyboard() != 0) {							// Check keyboard has been read - return not pressed on error
			return 0;
		}
		return SDL_KEY_DOWN(nCode);							// Return key state
	}

	if (nCode < 0x4000) {
		return 0;
	}

	if (nCode < 0x8000) {
		// Codes 4000-8000 = Joysticks
		int nJoyNumber = (nCode - 0x4000) >> 8;

		// Find the joystick state in our array
		return JoystickState(nJoyNumber, nCode & 0xFF);
	}

	if (nCode < 0xC000) {
		// Codes 8000-C000 = Mouse
		if ((nCode - 0x8000) >> 8) {						// Only the system mouse is supported by SDL
			return 0;
		}
		if (ReadMouse() != 0) {								// Error polling the mouse
			return 0;
		}
		return CheckMouseState(nCode & 0xFF);
	}

	return 0;
}

// This function finds which key is pressed, and returns its code
int SDLinpFind(bool CreateBaseline)
{
	int nRetVal = -1;										// assume nothing pressed

	// check if any keyboard keys are pressed
	if (ReadKeyboard() == 0) {
		for (int i = 0; i < 0x100; i++) {
			if (SDL_KEY_DOWN(i) > 0) {
				nRetVal = i;
				goto End;
			}
		}
	}

	// Now check all the connected joysticks
	for (int i = 0; i < nJoystickCount; i++) {
		int j;
		if (ReadJoystick() != 0) {							// There was an error polling the joystick
			continue;
		}

		for (j = 0; j < 0x10; j++) {						// Axes
			int nDelta = JoyPrevAxes[(i << 3) + (j >> 1)] - SDLinpJoyAxis(i, (j >> 1));
			if (nDelta < -0x4000 || nDelta > 0x4000) {
				if (JoystickState(i, j)) {
					nRetVal = 0x4000 | (i << 8) | j;
					goto End;
				}
			}
		}

		for (j = 0x10; j < 0x20; j++) {						// POV hats
			if (JoystickState(i, j)) {
				nRetVal = 0x4000 | (i << 8) | j;
				goto End;
			}
		}

		for (j = 0x80; j < 0x80 + SDL_JoystickNumButtons(JoyList[i]); j++) {
			if (JoystickState(i, j)) {
				nRetVal = 0x4000 | (i << 8) | j;
				goto End;
			}
		}
	}

	// Now the mouse
	if (ReadMouse() == 0) {
		int nDeltaX, nDeltaY;

		for (unsigned int j = 0x80; j < 0x80 + 0x80; j++) {
			if (CheckMouseState(j)) {
				nRetVal = 0x8000 | j;
				goto End;
			}
		}

		nDeltaX = SDLinpMouseAxis(0, 0);
		nDeltaY = SDLinpMouseAxis(0, 1);
		if (abs(nDeltaX) < abs(nDeltaY)) {
			if (nDeltaY != 0) {
				return 0x8000 | 1;
			}
		} else {
			if (nDeltaX != 0) {
				return 0x8000 | 0;
			}
		}
	}

End:

	if (CreateBaseline) {
		for (int i = 0; i < nJoystickCount; i++) {
			for (int j = 0; j < 8; j++) {
				JoyPrevAxes[(i << 3) + j] = SDLinpJoyAxis(i, j);
			}
		}
	}

	return nRetVal;
}

int SDLinpGetControlName(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName)
{
	if (pszDeviceName) {
		pszDeviceName[0] = _T('\0');
	}
	if (pszControlName) {
		pszControlName[0] = _T('\0');
	}

	switch (nCode & 0xC000) {
		case 0x0000: {
			_tcscpy(pszDeviceName, _T("System keyboard"));

			break;
		}
		case 0x4000: {
			int i = (nCode >> 8) & 0x3F;

			if (i >= nJoystickCount) {				// This joystick isn't connected
				return 0;
			}
			_tsprintf(pszDeviceName, "%hs", SDL_JoystickName(i));

			break;
		}
		case 0x8000: {
			int i = (nCode >> 8) & 0x3F;

			if (i >= 1) {
				return 0;
			}
			_tcscpy(pszDeviceName, _T("System mouse"));

			break;
		}
	}

	return 0;
}

struct InputInOut InputInOutSDL = { SDLinpInit, SDLinpExit, SDLinpSetCooperativeLevel, SDLinpStart, SDLinpState, SDLinpJoyAxis, SDLinpMouseAxis, SDLinpFind, SDLinpGetControlName, NULL, _T("SDL input") };
