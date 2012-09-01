/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * SDLInputSystem.cpp
 * 
 * Implementation of SDL input system.
 */

#include "SDLInputSystem.h"
#include "Supermodel.h"

#include <vector>
using namespace std;

SDLKeyMapStruct CSDLInputSystem::s_keyMap[] =
{
	// General keys
	{ "BACKSPACE",				SDLK_BACKSPACE },
	{ "TAB",					SDLK_TAB },
	{ "CLEAR",					SDLK_CLEAR },
	{ "RETURN",					SDLK_RETURN },
	{ "PAUSE",					SDLK_PAUSE },
	{ "ESCAPE",					SDLK_ESCAPE },
	{ "SPACE",					SDLK_SPACE },
	{ "EXCLAIM",				SDLK_EXCLAIM },
	{ "DBLQUOTE",				SDLK_QUOTEDBL },
	{ "HASH",					SDLK_HASH },
	{ "DOLLAR",					SDLK_DOLLAR },
	{ "AMPERSAND",				SDLK_AMPERSAND },
	{ "QUOTE",					SDLK_QUOTE },
	{ "LEFTPAREN",				SDLK_LEFTPAREN },
	{ "RIGHTPAREN",				SDLK_RIGHTPAREN },
	{ "ASTERISK",				SDLK_ASTERISK },
	{ "PLUS",					SDLK_PLUS },
	{ "COMMA",					SDLK_COMMA },
	{ "MINUS",					SDLK_MINUS },
	{ "PERIOD",					SDLK_PERIOD },
	{ "SLASH",					SDLK_SLASH },
	{ "0",						SDLK_0 },
	{ "1",						SDLK_1 },
	{ "2",						SDLK_2 },
	{ "3",						SDLK_3 },
	{ "4",						SDLK_4 },
	{ "5",						SDLK_5 },
	{ "6",						SDLK_6 },
	{ "7",						SDLK_7 },
	{ "8",						SDLK_8 },
	{ "9",						SDLK_9 },
	{ "COLON",					SDLK_COLON },
	{ "SEMICOLON",				SDLK_SEMICOLON },
	{ "LESS",					SDLK_LESS },
	{ "EQUALS",					SDLK_EQUALS },
	{ "GREATER",				SDLK_GREATER },
	{ "QUESTION",				SDLK_QUESTION },
	{ "AT",						SDLK_AT },
	{ "LEFTBRACKET",			SDLK_LEFTBRACKET },
	{ "BACKSLASH",				SDLK_BACKSLASH },
	{ "RIGHTBRACKET",			SDLK_RIGHTBRACKET },
	{ "CARET",					SDLK_CARET },
	{ "UNDERSCORE",				SDLK_UNDERSCORE },
	{ "BACKQUOTE",				SDLK_BACKQUOTE },
	{ "A",						SDLK_a },
	{ "B",						SDLK_b },
	{ "C",						SDLK_c },
	{ "D",						SDLK_d },
	{ "E",						SDLK_e },
	{ "F",						SDLK_f },
	{ "G",						SDLK_g },
	{ "H",						SDLK_h },
	{ "I",						SDLK_i },
	{ "J",						SDLK_j },
	{ "K",						SDLK_k },
	{ "L",						SDLK_l },
	{ "M",						SDLK_m },
	{ "N",						SDLK_n },
	{ "O",						SDLK_o },
	{ "P",						SDLK_p },
	{ "Q",						SDLK_q },
	{ "R",						SDLK_r },
	{ "S",						SDLK_s },
	{ "T",						SDLK_t },
	{ "U",						SDLK_u },
	{ "V",						SDLK_v },
	{ "W",						SDLK_w },
	{ "X",						SDLK_x },
	{ "Y",						SDLK_y },
	{ "Z",						SDLK_z },
	{ "DEL",					SDLK_DELETE },
	
	// Keypad
	{ "KEYPAD0",				SDLK_KP0 },
	{ "KEYPAD1",				SDLK_KP1 },
	{ "KEYPAD2",				SDLK_KP2 },
	{ "KEYPAD3",				SDLK_KP3 },
	{ "KEYPAD4",				SDLK_KP4 },
	{ "KEYPAD5",				SDLK_KP5 },
	{ "KEYPAD6",				SDLK_KP6 },
	{ "KEYPAD7",				SDLK_KP7 },
	{ "KEYPAD8",				SDLK_KP8 },
	{ "KEYPAD9",				SDLK_KP9 },
	{ "KEYPADPERIOD",			SDLK_KP_PERIOD },
	{ "KEYPADDIVIDE",			SDLK_KP_DIVIDE },
	{ "KEYPADMULTIPLY",			SDLK_KP_MULTIPLY },
	{ "KEYPADMINUS",			SDLK_KP_MINUS },
	{ "KEYPADPLUS",				SDLK_KP_PLUS },
	{ "KEYPADENTER",			SDLK_KP_ENTER },
	{ "KEYPADEQUALS",			SDLK_KP_EQUALS },
	
	// Arrows + Home/End Pad
	{ "UP",						SDLK_UP },
	{ "DOWN",					SDLK_DOWN },
	{ "RIGHT",					SDLK_RIGHT },
	{ "LEFT",					SDLK_LEFT },
	{ "INSERT",					SDLK_INSERT },
	{ "HOME",					SDLK_HOME },
	{ "END",					SDLK_END },
	{ "PGUP",					SDLK_PAGEUP },
	{ "PGDN",					SDLK_PAGEDOWN },

	// Function Key
	{ "F1",						SDLK_F1 },
	{ "F2",						SDLK_F2 },
	{ "F3",						SDLK_F3 },
	{ "F4",						SDLK_F4 },
	{ "F5",						SDLK_F5 },
	{ "F6",						SDLK_F6 },
	{ "F7",						SDLK_F7 },
	{ "F8",						SDLK_F8 },
	{ "F9",						SDLK_F9 },
	{ "F10",					SDLK_F10 },
	{ "F11",					SDLK_F11 },
	{ "F12",					SDLK_F12 },
	{ "F13",					SDLK_F13 },
	{ "F14",					SDLK_F14 },
	{ "F15",					SDLK_F15 },
    
	// Modifier Keys  
	// Removed Numlock, Capslock and Scrollock as don't seem to be handled well by SDL
	//{ "NUMLOCK",				SDLK_NUMLOCK },
	//{ "CAPSLOCK",				SDLK_CAPSLOCK },
	//{ "SCROLLLOCK",				SDLK_SCROLLOCK },
	{ "RIGHTSHIFT",				SDLK_RSHIFT },
	{ "LEFTSHIFT",				SDLK_LSHIFT },
	{ "RIGHTCTRL",				SDLK_RCTRL },
	{ "LEFTCTRL",				SDLK_LCTRL },
	{ "RIGHTALT",				SDLK_RALT },
	{ "LEFTALT",				SDLK_LALT },
	{ "RIGHTMETA",				SDLK_RMETA },
	{ "LEFTMETA",				SDLK_LMETA },
	{ "RIGHTWINDOWS",			SDLK_RSUPER },
	{ "LEFTWINDOWS",			SDLK_LSUPER },
	{ "ALTGR",					SDLK_MODE },
	{ "COMPOSE",				SDLK_COMPOSE },
    
	// Other
	{ "HELP",					SDLK_HELP },
	{ "PRINT",					SDLK_PRINT },
	{ "SYSREQ",					SDLK_SYSREQ },
	{ "BREAK",					SDLK_BREAK },
	{ "MENU",					SDLK_MENU },
	{ "POWER",					SDLK_POWER },
	{ "EURO",					SDLK_EURO },
	{ "UNDO",					SDLK_UNDO }
};

CSDLInputSystem::CSDLInputSystem() : CInputSystem("SDL"), m_keyState(NULL), m_mouseX(0), m_mouseY(0), m_mouseZ(0), m_mouseButtons(0)
{
	//
}

CSDLInputSystem::~CSDLInputSystem()
{
	CloseJoysticks();
}

void CSDLInputSystem::OpenJoysticks()
{
	// Open all available joysticks
	int numJoys = SDL_NumJoysticks();
	for (int joyNum = 0; joyNum < numJoys; joyNum++)
	{
		SDL_Joystick *joystick = SDL_JoystickOpen(joyNum);
		if (joystick == NULL)
		{
			ErrorLog("Unable to open joystick device %d with SDL - skipping joystick.\n", joyNum + 1);
			continue;
		}
		
		// Gather joystick details (name, num POVs & buttons and which axes are available)
		JoyDetails joyDetails;
		const char *pName = SDL_JoystickName(joyNum);
		strncpy(joyDetails.name, pName, MAX_NAME_LENGTH);
		joyDetails.name[MAX_NAME_LENGTH] = '\0';
		joyDetails.numAxes = SDL_JoystickNumAxes(joystick);
		for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
		{
			joyDetails.hasAxis[axisNum] = joyDetails.numAxes > axisNum;
			joyDetails.axisHasFF[axisNum] = false; // SDL 1.2 does not support force feedback
			char *axisName = joyDetails.axisName[axisNum];
			strcpy(axisName, CInputSystem::GetDefaultAxisName(axisNum)); // SDL 1.2 does not support axis names
			strcat(axisName, "-Axis");
		}
		joyDetails.numPOVs = SDL_JoystickNumHats(joystick);
		joyDetails.numButtons = SDL_JoystickNumButtons(joystick);
		joyDetails.hasFFeedback = false; // SDL 1.2 does not support force feedback
		
		m_joysticks.push_back(joystick);
		m_joyDetails.push_back(joyDetails);
	}
}

void CSDLInputSystem::CloseJoysticks()
{
	// Close all previously opened joysticks
	for (size_t i = 0; i < m_joysticks.size(); i++)
	{	
		SDL_Joystick *joystick = m_joysticks[i];
		SDL_JoystickClose(joystick);
	}

	m_joysticks.clear();
	m_joyDetails.clear();
}

bool CSDLInputSystem::InitializeSystem()
{
	// Make sure joystick subsystem is initialized and joystick events are enabled
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
	{
		ErrorLog("Unable to initialize SDL joystick subsystem (%s).\n", SDL_GetError());

		return false;
	}
	SDL_JoystickEventState(SDL_ENABLE);

	// Open attached joysticks
	OpenJoysticks();
	return true;
}

int CSDLInputSystem::GetKeyIndex(const char *keyName)
{
	for (int i = 0; i < NUM_SDL_KEYS; i++)
	{
		if (stricmp(keyName, s_keyMap[i].keyName) == 0)
			return i;
	}
	return -1;
}

const char *CSDLInputSystem::GetKeyName(int keyIndex)
{
	if (keyIndex < 0 || keyIndex >= NUM_SDL_KEYS)
		return NULL;
	return s_keyMap[keyIndex].keyName;
}

bool CSDLInputSystem::IsKeyPressed(int kbdNum, int keyIndex)
{
	// Get SDL key for given index and check if currently pressed
	SDLKey sdlKey = s_keyMap[keyIndex].sdlKey;
	return !!m_keyState[sdlKey];
}

int CSDLInputSystem::GetMouseAxisValue(int mseNum, int axisNum)
{
	// Return value for given mouse axis
	switch (axisNum)
	{
		case AXIS_X: return m_mouseX;
		case AXIS_Y: return m_mouseY;
		case AXIS_Z: return m_mouseZ;
		default:     return 0;
	}
}

int CSDLInputSystem::GetMouseWheelDir(int mseNum)
{
	// Return wheel value
	return m_mouseWheelDir;
}

bool CSDLInputSystem::IsMouseButPressed(int mseNum, int butNum)
{
	// Return value for given mouse button
	switch (butNum)
	{
		case 0:  return !!(m_mouseButtons & SDL_BUTTON_LMASK);
		case 1:  return !!(m_mouseButtons & SDL_BUTTON_MMASK);
		case 2:  return !!(m_mouseButtons & SDL_BUTTON_RMASK);
		case 3:  return !!(m_mouseButtons & SDL_BUTTON_X1MASK);
		case 4:  return !!(m_mouseButtons & SDL_BUTTON_X2MASK);
		default: return false;
	}
}

int CSDLInputSystem::GetJoyAxisValue(int joyNum, int axisNum)
{
	// Get raw joystick axis value for given joystick from SDL (values range from -32768 to 32767)
	SDL_Joystick *joystick = m_joysticks[joyNum];
	return SDL_JoystickGetAxis(joystick, axisNum);
}

bool CSDLInputSystem::IsJoyPOVInDir(int joyNum, int povNum, int povDir)
{
	// Get current joystick POV-hat value for given joystick and POV number from SDL and check if pointing in required direction
	SDL_Joystick *joystick = m_joysticks[joyNum];
	int hatVal = SDL_JoystickGetHat(joystick, povNum);
	switch (povDir)
	{
		case POV_UP:    return !!(hatVal & SDL_HAT_UP);
		case POV_DOWN:  return !!(hatVal & SDL_HAT_DOWN);
		case POV_LEFT:  return !!(hatVal & SDL_HAT_LEFT);
		case POV_RIGHT: return !!(hatVal & SDL_HAT_RIGHT);
		default:        return false;
	}
	return false;
}

bool CSDLInputSystem::IsJoyButPressed(int joyNum, int butNum)
{
	// Get current joystick button state for given joystick and button number from SDL
	SDL_Joystick *joystick = m_joysticks[joyNum];
	return !!SDL_JoystickGetButton(joystick, butNum);
}

bool CSDLInputSystem::ProcessForceFeedbackCmd(int joyNum, int axisNum, ForceFeedbackCmd ffCmd)
{
	// SDL 1.2 does not support force feedback
	return false;
}

void CSDLInputSystem::Wait(int ms)
{
	SDL_Delay(ms);
}

int CSDLInputSystem::GetNumKeyboards()
{
	// Return ANY_KEYBOARD as SDL 1.2 cannot handle multiple keyboards
	return ANY_KEYBOARD;
}
	
int CSDLInputSystem::GetNumMice()
{
	// Return ANY_MOUSE as SDL 1.2 cannot handle multiple mice
	return ANY_MOUSE;
}
	
int CSDLInputSystem::GetNumJoysticks()
{
	// Return number of joysticks found
	return m_joysticks.size();
}

const KeyDetails *CSDLInputSystem::GetKeyDetails(int kbdNum)
{
	// Return NULL as SDL 1.2 cannot handle multiple keyboards
	return NULL;
}

const MouseDetails *CSDLInputSystem::GetMouseDetails(int mseNum)
{
	// Return NULL as SDL 1.2 cannot handle multiple mice
	return NULL;
}

const JoyDetails *CSDLInputSystem::GetJoyDetails(int joyNum)
{
	return &m_joyDetails[joyNum];
}

bool CSDLInputSystem::Poll()
{
	// Reset mouse wheel direction
	m_mouseWheelDir = 0;

	// Poll for event from SDL
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			return false; 	
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			// Mouse wheel movements are not returned by SDL_GetMouseState below, so they are handled here instead
			// The mouse absolute Z-axis value and mouse wheel direction value are updated appropriately
			if (e.button.button == SDL_BUTTON_WHEELUP)
			{
				m_mouseZ += 5;
				m_mouseWheelDir = 1;
			}
			else if (e.button.button == SDL_BUTTON_WHEELDOWN)
			{
				m_mouseZ -= 5;
				m_mouseWheelDir = -1;
			}
		}
	}

	// Get key state from SDL
	m_keyState = SDL_GetKeyState(NULL);

	// Get mouse state from SDL (except mouse wheel which was handled earlier)
	m_mouseButtons = SDL_GetMouseState(&m_mouseX, &m_mouseY);

	// Update joystick state (not required as called implicitly by SDL_PollEvent above)
	//SDL_JoystickUpdate();
	return true;
}

void CSDLInputSystem::SetMouseVisibility(bool visible)
{
	SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);  	
}
