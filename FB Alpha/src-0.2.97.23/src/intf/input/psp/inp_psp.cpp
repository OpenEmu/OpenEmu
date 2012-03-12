// Module for input
#include "burner.h"

#define MAX_JOYSTICKS (1)

static int nInitedSubsytems = 0;
static int* JoyPrevAxes = NULL;
static int nJoystickCount = 0;						// Number of joysticks connected to this machine

static SceCtrlData myInput;

int PSPinpSetCooperativeLevel(bool bExclusive, bool /*bForeGround*/)
{
	return 0;
}

int PSPinpExit()
{
	nJoystickCount = 0;

	free(JoyPrevAxes);
	JoyPrevAxes = NULL;

	return 0;
}

int PSPinpInit()
{
	int nSize;

	nSize = MAX_JOYSTICKS * 8 * sizeof(int);
	if ((JoyPrevAxes = (int*)malloc(nSize)) == NULL) {
		return 1;
	}
	memset(JoyPrevAxes, 0, nSize);
	// Set up the joysticks
	nJoystickCount = 1;
	return 0;
}

static unsigned char bKeyboardRead = 0;

static unsigned char bJoystickRead = 0;

static unsigned char bMouseRead = 0;
static struct { unsigned char buttons; int xdelta; int ydelta; } PSPinpMouseState;

// Call before checking for Input in a frame
int PSPinpStart()
{
	// Keyboard not read this frame
	bKeyboardRead = 0;

	// No joysticks have been read for this frame
	bJoystickRead = 0;

	// Mouse not read this frame
	bMouseRead = 0;

	sceCtrlPeekBufferPositive( &myInput, 1 );

	return 0;
}

// Read one of the joysticks
static int ReadJoystick()
{
	if (bJoystickRead) {
		return 0;
	}

	// All joysticks have been Read this frame
	bJoystickRead = 1;

	return 0;
}

// Read one joystick axis
int PSPinpJoyAxis(int i, int nAxis)
{
	if (i < 0 || i >= nJoystickCount) {				// This joystick number isn't connected
		return 0;
	}

	if (ReadJoystick() != 0) {						// There was an error polling the joystick
		return 0;
	}

	return 0;
}

// Read the keyboard
static int ReadKeyboard()
{
	int numkeys;

	if (bKeyboardRead) {							// already read this frame - ready to go
		return 0;
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

	bMouseRead = 1;

	return 0;
}

// Read one mouse axis
int PSPinpMouseAxis(int i, int nAxis)
{
	if (i < 0 || i >= 1) {									// Only the system mouse is supported by SDL
		return 0;
	}

	return 0;
}

// Check a subcode (the 40xx bit in 4001, 4102 etc) for a joystick input code
static int JoystickState(int i, int nSubCode)
{
	return 0;
}

// Check a subcode (the 80xx bit in 8001, 8102 etc) for a mouse input code
static int CheckMouseState(unsigned int nSubCode)
{

	return 0;
}

// Get the state (pressed = 1, not pressed = 0) of a particular input code
// look in inp_keys.h to see what keycodes need mapping, e.g. 9 is used for the
// diag mode in cps, I think. Better check on the win32 version :-)
int PSPinpState(int nCode)
{
	switch(nCode)
	{
	case 0x02: // start
		return ((myInput.Buttons & PSP_CTRL_START) ? 1 : 0);
		break;
	case 0x06: // coin 
		return ((myInput.Buttons & PSP_CTRL_SELECT) ? 1 : 0);
		break;
	case 0x3c: //f2
		break;
	case 0x3D: //f3
		break;
	case 0x4000: // left
		return ((myInput.Buttons & PSP_CTRL_LEFT) ? 1 : 0);
		break;
	case 0x4001: // right 
		return ((myInput.Buttons & PSP_CTRL_RIGHT) ? 1 : 0);
		break;
	case 0x4002: // up 
		return ((myInput.Buttons & PSP_CTRL_UP) ? 1 : 0);
		break;
	case 0x4003: // down 
		return ((myInput.Buttons & PSP_CTRL_DOWN) ? 1 : 0);
		break;
	
	case 0x4080: //fire1 
		return ((myInput.Buttons & PSP_CTRL_SQUARE) ? 1 : 0);
		break;
	case 0x4081: //fire 2 
		return ((myInput.Buttons & PSP_CTRL_CROSS) ? 1 : 0);
		break;
	case 0x4082: // etc 
		return ((myInput.Buttons & PSP_CTRL_CIRCLE) ? 1 : 0);
		break;
	case 0x4083: 
		return ((myInput.Buttons & PSP_CTRL_TRIANGLE) ? 1 : 0);
		break;
	case 0x4084:
		return ((myInput.Buttons & PSP_CTRL_LTRIGGER) ? 1 : 0);
		break;
	case 0x4085: 
		return ((myInput.Buttons & PSP_CTRL_RTRIGGER) ? 1 : 0);
		break;
	//fba can map 2 more buttons but I dont think the psp has that many and no games use that much that I know of
	default:
		return 0;
	}
	return 0;
}

// This function finds which key is pressed, and returns its code
int PSPinpFind(bool CreateBaseline)
{
	int nRetVal = -1;										// assume nothing pressed

	return nRetVal;
}

int PSPinpGetControlName(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName)
{
	return 0;
}

struct InputInOut InputInOutPSP = { PSPinpInit, PSPinpExit, PSPinpSetCooperativeLevel, PSPinpStart, PSPinpState, PSPinpJoyAxis, PSPinpMouseAxis, PSPinpFind, PSPinpGetControlName, NULL, _T("PSP input") };
